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
#pragma once

#include <string>
#include <list>

#include <tinyxml.h>

#include <tiny/mesh/animatedmesh.h>
#include <tiny/draw/animatedmeshhorde.h>
#include <tiny/draw/texture2d.h>

#include "terrain.h"

namespace moba
{

class MinionType
{
    public:
        MinionType(const std::string &, TiXmlElement *);
        ~MinionType();
        
        void updateInstances();
        
        std::string name;
        
        float maxSpeed;
        float radius;
        
        int maxNrInstances;
        tiny::mesh::AnimatedMesh mesh;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        tiny::draw::AnimationTextureBuffer *animationTexture;
        tiny::draw::AnimatedMeshHorde *horde;
        
        std::list<tiny::draw::AnimatedMeshInstance> instances;
};

class MinionPath
{
    public:
        MinionPath(const std::string &, TiXmlElement *);
        ~MinionPath();
        
        void plantNodes(const GameTerrain *);
        
        std::string name;
        std::vector<tiny::vec2> nodes;
};

class Minion
{
    public:
        Minion(const std::string &, const std::string &, const tiny::vec2 &);
        ~Minion();
        
        std::string name;
        std::string type;
        
        std::string path;
        unsigned int pathIndex;
        
        tiny::vec2 pos;
        float angle;
        
        std::string action;
        float actionTime;
};

} //namespace moba

