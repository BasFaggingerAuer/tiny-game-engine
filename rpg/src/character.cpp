/*
Copyright 2020, Bas Fagginger Auer.

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
#include <vector>
#include <exception>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>

#include "character.h"

using namespace rpg;
using namespace tiny;

CharacterType::CharacterType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading character type resource..." << std::endl;
   
    assert(std::string(el->Value()) == "charactertype");
    
    name = "";
    size = vec3(1.0f);
    float scaleHeight = 0.125f;
    
    nrInstances = 0;
    maxNrInstances = 1;
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("width", &size.x);
    el->QueryFloatAttribute("height", &size.y);
    el->QueryFloatAttribute("depth", &size.z);
    el->QueryFloatAttribute("scale_height", &scaleHeight);
    
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read mesh and textures.
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createUpNormalImage() : img::io::readImage(path + normalFileName));
    mesh::StaticMesh mesh = (meshFileName.empty() ? mesh::StaticMesh::createBoxMesh(1.0f, 1.0f, 1.0f) : mesh::io::readStaticMesh(path + meshFileName));
    
    //Center mesh and fit it to size.
    const std::pair<vec3, vec3> boundingBox = mesh.getBoundingBox();
    
    //Extract bottom 12.5% of the mesh to use for centering and scaling.
    mesh::StaticMesh lowerMesh;
    const float lowerY = boundingBox.first.y + (boundingBox.second.y - boundingBox.first.y)*scaleHeight;
    
    for (auto i = mesh.vertices.cbegin(); i != mesh.vertices.cend(); ++i)
    {
        if (i->position.y <= lowerY)
        {
            lowerMesh.vertices.push_back(*i);
        }
    }
    
    const std::pair<vec3, vec3> lowerBoundingBox = lowerMesh.getBoundingBox();
    const vec3 o = vec3(0.5f*(lowerBoundingBox.first.x + lowerBoundingBox.second.x), lowerBoundingBox.first.y, 0.5f*(lowerBoundingBox.first.z + lowerBoundingBox.second.z));
    const float s = minComponent(abs(size.xz() - 0.1f)/abs(lowerBoundingBox.second.xz() - lowerBoundingBox.first.xz()));
    
    for (std::vector<mesh::StaticMeshVertex>::iterator i = mesh.vertices.begin(); i != mesh.vertices.end(); ++i)
    {
        i->position = s*(i->position - o);
    }
    
    horde = new draw::StaticMeshHorde(mesh, maxNrInstances);
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    shadowDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    shadowNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    
    shadowHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createBoxMesh(0.5f*size.x - 0.05f, 0.05f, 0.5f*size.z - 0.05f), maxNrInstances);
    shadowHorde->setDiffuseTexture(*shadowDiffuseTexture);
    shadowHorde->setNormalTexture(*shadowNormalTexture);
    shadowInstances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' character type." << std::endl;
}

CharacterType::~CharacterType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
    
    delete shadowHorde;
    delete shadowDiffuseTexture;
    delete shadowNormalTexture;
}

void CharacterType::clearInstances()
{
    nrInstances = 0;
}

void CharacterType::addInstance(const CharacterInstance &instance, const float &baseHeight)
{
    if (nrInstances < maxNrInstances)
    {
        vec4 r = quatrot((M_PI/180.0f)*static_cast<float>(instance.rotation), vec3(0.0f, 1.0f, 0.0f));
        
        if (instance.state == 1)
        {
            r = quatmul(r, quatrot(0.5f*M_PI, vec3(1.0f, 0.0f, 0.0f)));
        }
        
        instances[nrInstances] = draw::StaticMeshInstance(vec4(static_cast<float>(instance.position.x + 1) - 0.5f*size.x,
                                                               static_cast<float>(instance.position.y + baseHeight),
                                                               static_cast<float>(instance.position.z + 1) - 0.5f*size.z,
                                                               1.0f),
                                                            r,
                                                            instance.getColor());
        shadowInstances[nrInstances] = draw::StaticMeshInstance(vec4(static_cast<float>(instance.position.x + 1) - 0.5f*size.x,
                                                               static_cast<float>(baseHeight),
                                                               static_cast<float>(instance.position.z + 1) - 0.5f*size.z,
                                                               1.0f),
                                                            vec4(0.0f, 0.0f, 0.0f, 1.0f),
                                                            instance.getColor());
        ++nrInstances;
    }
}

void CharacterType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
    shadowHorde->setMeshes(shadowInstances.begin(), shadowInstances.begin() + nrInstances);
}

