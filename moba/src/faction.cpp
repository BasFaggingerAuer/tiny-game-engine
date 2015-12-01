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
#include <list>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>
#include <tiny/mesh/io/animatedmesh.h>

#include "faction.h"

using namespace moba;
using namespace tiny;

Faction::Faction(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading faction resources..." << std::endl;
    
    assert(el);
    
    nexusScale = 1.0f;
    nexusMesh = 0;
    nexusPosition = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    towerScale = 1.0f;
    
    assert(el->ValueStr() == "faction");
    
    el->QueryStringAttribute("name", &name);
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "nexus")
        {
            img::Image diffuseImage = img::Image::createSolidImage();
            img::Image normalImage = img::Image::createUpNormalImage();
            mesh::StaticMesh mesh = mesh::StaticMesh::createCubeMesh();

            if (nexusMesh)
            {
                std::cerr << "The nexus has already been defined!" << std::endl;
                throw std::exception();
            }
            
            sl->QueryFloatAttribute("scale", &nexusScale);
            sl->QueryFloatAttribute("x", &nexusPosition.x);
            sl->QueryFloatAttribute("y", &nexusPosition.y);
            sl->QueryFloatAttribute("dz", &nexusPosition.z);
            sl->QueryFloatAttribute("r", &nexusPosition.w);
            
            if (sl->Attribute("mesh")) mesh = mesh::io::readStaticMesh(path + std::string(sl->Attribute("mesh")));
            if (sl->Attribute("diffuse")) diffuseImage = img::io::readImage(path + std::string(sl->Attribute("diffuse")));
            if (sl->Attribute("normal")) normalImage = img::io::readImage(path + std::string(sl->Attribute("normal")));
            
            //High-detail meshes.
            nexusDiffuseTexture = new draw::RGBATexture2D(diffuseImage);
            nexusNormalTexture = new draw::RGBATexture2D(normalImage);
            nexusMesh = new draw::StaticMeshHorde(mesh, 1);
            nexusMesh->setDiffuseTexture(*nexusDiffuseTexture);
            nexusMesh->setNormalTexture(*nexusNormalTexture);
            
        }
        else if (sl->ValueStr() == "towers")
        {
            img::Image diffuseImage = img::Image::createSolidImage();
            img::Image normalImage = img::Image::createUpNormalImage();
            mesh::StaticMesh mesh = mesh::StaticMesh::createCubeMesh();

            if (!towerPositions.empty())
            {
                std::cerr << "Towers have already been defined!" << std::endl;
                throw std::exception();
            }
            
            sl->QueryFloatAttribute("scale", &towerScale);
            
            if (sl->Attribute("mesh")) mesh = mesh::io::readStaticMesh(path + std::string(sl->Attribute("mesh")));
            if (sl->Attribute("diffuse")) diffuseImage = img::io::readImage(path + std::string(sl->Attribute("diffuse")));
            if (sl->Attribute("normal")) normalImage = img::io::readImage(path + std::string(sl->Attribute("normal")));
            
            for (TiXmlElement *tl = sl->FirstChildElement(); tl; tl = tl->NextSiblingElement())
            {
                if (tl->ValueStr() == "instance")
                {
                    float x = 0.0f;
                    float y = 0.0f;
                    float dz = 0.0f;
                    float r = 0.0f;
                    
                    tl->QueryFloatAttribute("x", &x);
                    tl->QueryFloatAttribute("y", &y);
                    tl->QueryFloatAttribute("dz", &dz);
                    tl->QueryFloatAttribute("r", &r);
                    towerPositions.push_back(vec4(x, y, dz, r));
                }
            }
            
            if (towerPositions.empty())
            {
                std::cerr << "Need at least one tower!" << std::endl;
                throw std::exception();
            }
            
            //High-detail meshes.
            towerDiffuseTexture = new draw::RGBATexture2D(diffuseImage);
            towerNormalTexture = new draw::RGBATexture2D(normalImage);
            towerMeshes = new draw::StaticMeshHorde(mesh, towerPositions.size());
            towerMeshes->setDiffuseTexture(*towerDiffuseTexture);
            towerMeshes->setNormalTexture(*towerNormalTexture);
        }
    }
    
}

Faction::~Faction()
{
    delete nexusMesh;
    delete nexusNormalTexture;
    delete nexusDiffuseTexture;
    
    delete towerMeshes;
    delete towerNormalTexture;
    delete towerDiffuseTexture;
}

void Faction::plantBuildings(const GameTerrain *terrain)
{
    std::list<draw::StaticMeshInstance> instances;
    vec3 pos;
    
    //Nexus.
    pos = terrain->getWorldPosition(nexusPosition.x, nexusPosition.y);
    instances.push_back(draw::StaticMeshInstance(vec4(pos.x, pos.y + nexusPosition.z, pos.z, nexusScale), quatrot(nexusPosition.w, vec3(0.0f, 1.0f, 0.0f))));
    nexusMesh->setMeshes(instances.begin(), instances.end());
    
    //Towers.
    instances.clear();
    
    for (std::list<vec4>::const_iterator i = towerPositions.begin(); i != towerPositions.end(); ++i)
    {
        pos = terrain->getWorldPosition(i->x, i->y);
        instances.push_back(draw::StaticMeshInstance(vec4(pos.x, pos.y + i->z, pos.z, towerScale), quatrot(i->w, vec3(0.0f, 1.0f, 0.0f))));
    }
    
    towerMeshes->setMeshes(instances.begin(), instances.end());
}


