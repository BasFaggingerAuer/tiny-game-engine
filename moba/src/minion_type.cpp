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
#include <exception>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/animatedmesh.h>

#include "minion_type.h"

using namespace moba;
using namespace tiny;

MinionType::MinionType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading minion type resources..." << std::endl;
    
    assert(el);
    assert(el->ValueStr() == "minion_type");
    
    img::Image diffuseImage = img::Image::createSolidImage();
    img::Image normalImage = img::Image::createUpNormalImage();

    name = "Unspecified";
    maxSpeed = 1.0f;
    radius = 1.0f;
    maxNrInstances = 1024;
    
    el->QueryIntAttribute("nr_instances", &maxNrInstances);
    el->QueryFloatAttribute("max_speed", &maxSpeed);
    el->QueryFloatAttribute("radius", &radius);
    
    if (el->Attribute("name")) name = std::string(el->Attribute("name"));
    if (el->Attribute("mesh")) mesh = mesh::io::readAnimatedMesh(path + std::string(el->Attribute("mesh")));
    if (el->Attribute("diffuse")) diffuseImage = img::io::readImage(path + std::string(el->Attribute("diffuse")));
    if (el->Attribute("normal")) normalImage = img::io::readImage(path + std::string(el->Attribute("normal")));
    
    //Create textures.
    diffuseTexture = new draw::RGBTexture2D(diffuseImage);
    normalTexture = new draw::RGBTexture2D(normalImage);
    animationTexture = new draw::AnimationTextureBuffer();
    animationTexture->setAnimations(mesh.skeleton.animations.begin(), mesh.skeleton.animations.end());
    
    //Create horde.
    horde = new draw::AnimatedMeshHorde(mesh, maxNrInstances);
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    horde->setAnimationTexture(*animationTexture);
    instances.clear();
}

MinionType::~MinionType()
{
    delete horde;
    delete animationTexture;
    delete normalTexture;
    delete diffuseTexture;
}

MinionPath::MinionPath(const std::string &, TiXmlElement *el) :
    name("Unspecified"),
    nodes()
{
    std::cerr << "Reading minion path..." << std::endl;
    
    assert(el->ValueStr() == "minion_path");
    
    el->QueryStringAttribute("name", &name);
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "node")
        {
            vec2 pos(0.0f, 0.0f);
            
            sl->QueryFloatAttribute("x", &pos.x);
            sl->QueryFloatAttribute("y", &pos.y);
            nodes.push_back(pos);
        }
    }
    
    if (nodes.empty())
    {
        std::cerr << "Empty path defined!" << std::endl;
        throw std::exception();
    }
    
    std::cerr << "Added minion path with " << nodes.size() << " nodes." << std::endl;
}

MinionPath::~MinionPath()
{
    
}

void MinionPath::plantNodes(const GameTerrain *terrain)
{
    //Convert path nodes to world coordinates for the current terrain.
    assert(terrain);
    
    for (std::vector<vec2>::iterator i = nodes.begin(); i != nodes.end(); ++i)
    {
        const vec3 pos = terrain->getWorldPosition(*i);
        
        *i = vec2(pos.x, pos.z);
    }
}

void MinionType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.end());
}

Minion::Minion(const std::string &a_name, const std::string &a_type, const vec2 &a_pos) :
                name(a_name),
                type(a_type),
                path(""),
                pathIndex(0),
                pos(a_pos),
                angle(0.0f),
                action(""),
                actionTime(0.0f)
{
    
}

Minion::~Minion()
{

}

