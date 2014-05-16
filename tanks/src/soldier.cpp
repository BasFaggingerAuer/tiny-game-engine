/*
Copyright 2012, Bas Fagginger Auer.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>
#include <tiny/snd/io/sample.h>

#include "soldier.h"

using namespace tanks;
using namespace tiny;

ExplosionType::ExplosionType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading explosion type resource..." << std::endl;
    
    assert(el->ValueStr() == "explosion");
    
    name = "";
    minRadius = 0.0f;
    maxRadius = 1.0f;
    expansionSpeed = 1.0f;
    push = 0.0f;
    damage = 0.0f;
    explodeSound = 0;
    std::string emitFileName = "";
    std::string explodeSoundFileName = "";
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("min_radius", &minRadius);
    el->QueryFloatAttribute("max_radius", &maxRadius);
    el->QueryStringAttribute("emit", &emitFileName);
    el->QueryFloatAttribute("expansion_speed", &expansionSpeed);
    el->QueryFloatAttribute("push", &push);
    el->QueryFloatAttribute("damage", &damage);
    el->QueryStringAttribute("explode_sound", &explodeSoundFileName);
    
    explodeImage = new tiny::img::Image(emitFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + emitFileName));
    
    if (!explodeSoundFileName.empty()) explodeSound = snd::io::readSample(path + explodeSoundFileName);
    
    icon = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

ExplosionType::~ExplosionType()
{
    if (explodeSound) delete explodeSound;
}

BulletType::BulletType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading bullet type resource..." << std::endl;
    
    assert(el->ValueStr() == "bullet");
    
    name = "";
    radius = 0.1f;
    position = vec3(0.0f, 0.0f, 0.0f);
    velocity = vec3(0.0f, 0.0f, -1.0f);
    acceleration = vec3(0.0f, 0.0f, 0.0f);
    shootSound = 0;
    std::string emitFileName = "";
    std::string shootSoundFileName = "";
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("radius", &radius);
    el->QueryStringAttribute("emit", &emitFileName);
    el->QueryFloatAttribute("lifetime", &lifetime);
    el->QueryFloatAttribute("vel_x", &velocity.x);
    el->QueryFloatAttribute("vel_y", &velocity.y);
    el->QueryFloatAttribute("vel_z", &velocity.z);
    el->QueryFloatAttribute("acc_x", &acceleration.x);
    el->QueryFloatAttribute("acc_y", &acceleration.y);
    el->QueryFloatAttribute("acc_z", &acceleration.z);
    el->QueryStringAttribute("shoot_sound", &shootSoundFileName);
    
    bulletImage = new tiny::img::Image(emitFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + emitFileName));
    
    if (!shootSoundFileName.empty()) shootSound = snd::io::readSample(path + shootSoundFileName);
    
    icon = vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

BulletType::~BulletType()
{
    delete bulletImage;
    if (shootSound) delete shootSound;
}

SoldierWeapon::SoldierWeapon(const std::string &, TiXmlElement *el)
{
    assert(el->ValueStr() == "weapon");
    
    name = "";
    rechargeTime = 1.0f;
    bulletName = "";
    bulletType = 0;
    explosionName = "";
    explosionType = 0;
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("recharge_time", &rechargeTime);
    el->QueryStringAttribute("bullet", &bulletName);
    el->QueryStringAttribute("explosion", &explosionName);
    
    std::cerr << "Added '" << name << "' soldier weapon type." << std::endl;
}

SoldierWeapon::~SoldierWeapon()
{
    
}

SoldierType::SoldierType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading soldier type resource..." << std::endl;
   
    assert(el->ValueStr() == "soldier");
    
    name = "";
    radius = 1.0f;
    height = 1.0f;
    mass = 1.0f;
    jump = 1.0f;
    speed = 1.0f;
    cameraPosition = vec3(0.0f, 1.0f, 0.0f);
    
    jumpjet = false;
    jumpjetThrust = 0.0f;
    jumpjetFuel = 1.0f;
    jumpjetCharge = 1.0f;
    
    nrInstances = 0;
    maxNrInstances = 1;
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("radius", &radius);
    el->QueryFloatAttribute("mass", &mass);
    el->QueryFloatAttribute("jump", &jump);
    el->QueryFloatAttribute("speed", &speed);
    el->QueryFloatAttribute("camera_height", &cameraPosition.y);
    
    /*
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    */
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read thrusters.
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
             if (sl->ValueStr() == "jumpjet")
        {
            jumpjet = true;
            sl->QueryFloatAttribute("thrust", &jumpjetThrust);
            sl->QueryFloatAttribute("fuel", &jumpjetFuel);
            sl->QueryFloatAttribute("charge", &jumpjetCharge);
        }
        else if (sl->ValueStr() == "weapon")
        {
            weapons.push_back(SoldierWeapon(path, sl));
        }
    }
    
    //Read mesh and textures.
    /*
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + normalFileName));
    horde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(path + meshFileName), maxNrInstances);
    */
    diffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    normalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    horde = new draw::StaticMeshHorde(mesh::StaticMesh::createCylinderMesh(radius, height), maxNrInstances);
    
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' soldier type." << std::endl;
}

SoldierType::~SoldierType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
}

vec3 SoldierType::getCameraPosition(const SoldierInstance &soldier) const
{
    //Return the soldier's camera position.
    return soldier.x + cameraPosition;
}

vec4 SoldierType::getCameraOrientation(const SoldierInstance &soldier) const
{
    //Return the soldier's camera orientation.
    return quatmul(quatrot(soldier.angles.y, vec3(1.0f, 0.0f, 0.0f)), quatrot(soldier.angles.x, vec3(0.0f, 1.0f, 0.0f)));
}

void SoldierType::clearInstances()
{
    nrInstances = 0;
}

void SoldierType::addInstance(const SoldierInstance &soldier)
{
    if (nrInstances < maxNrInstances)
    {
        instances[nrInstances++] = draw::StaticMeshInstance(vec4(soldier.x.x, soldier.x.y, soldier.x.z, 1.0f), soldier.q);
    }
}

void SoldierType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
}

