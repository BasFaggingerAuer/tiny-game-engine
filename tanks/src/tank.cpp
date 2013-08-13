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

#include "tank.h"

using namespace tanks;
using namespace tiny;

TankType::TankType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading tank type resource..." << std::endl;
   
    assert(el->ValueStr() == "tank");
    
    name = "";
    mass = 1.0f;
    radius1 = 1.0f;
    radius2 = 2.0f;
    
    nrInstances = 0;
    maxNrInstances = 0;
     
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("mass", &mass);
    el->QueryFloatAttribute("solid_radius", &radius1);
    el->QueryFloatAttribute("shield_radius", &radius2);
    
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read thrusters.
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "thruster")
        {
            std::string id = "";
            std::istringstream stream(sl->GetText());
            int i = 0;
            
            sl->QueryStringAttribute("id", &id);
            
                 if (id == "left") i = 0;
            else if (id == "right") i = 1;
            else if (id == "up") i = 2;
            else if (id == "down") i = 3;
            else i = -1;
            
            if (i >= 0 && i < 4)
            {
                stream >> thrust_pos[i].x >> thrust_pos[i].y >> thrust_pos[i].z;
                stream >> thrust_force[i].x >> thrust_force[i].y >> thrust_force[i].z;
            }
            else
            {
                std::cerr << "Unknown thruster id '" << id << "'!" << std::endl;
            }
        }
    }
    
    //Inertia for a solid sphere (2/5)*m*r^2.
    inertia = 0.4f*mass*radius1*radius1;
    
    //Read mesh and textures.
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + normalFileName));
    horde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(path + meshFileName), maxNrInstances);
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' tank type." << std::endl;
}

TankType::~TankType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
}

void TankType::clearInstances()
{
    nrInstances = 0;
}

void TankType::addInstance(const TankInstance &tank)
{
    if (nrInstances < maxNrInstances)
    {
        instances[nrInstances++] = draw::StaticMeshInstance(vec4(tank.x.x, tank.x.y, tank.x.z, 1.0f), tank.q);
    }
}

void TankType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
}

