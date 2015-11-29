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
    maxNrInstances = 1024;
    
    el->QueryIntAttribute("nr_instances", &maxNrInstances);
    
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

void MinionType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.end());
}

Minion::Minion(const std::string &a_name, const std::string &a_type) : name(a_name), type(a_type), pos(0.0f, 0.0f), angle(0.0f), action(""), actionTime(0.0f)
{
    
}

Minion::~Minion()
{

}

