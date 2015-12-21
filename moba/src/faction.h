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

#include <tiny/mesh/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/mesh/animatedmesh.h>
#include <tiny/draw/animatedmeshhorde.h>
#include <tiny/draw/texture2d.h>

#include "terrain.h"

namespace moba
{

class MinionSpawner
{
    public:
        MinionSpawner(const std::string &, TiXmlElement *);
        ~MinionSpawner();
        
        std::string minionType;
        std::string pathName;
        int nrSpawn;
        float radius;
        float cooldownTime;
        float currentTime;
};

class Faction
{
    public:
        Faction(const std::string &, TiXmlElement *);
        ~Faction();
        
        std::list<tiny::vec4> plantBuildings(const GameTerrain *terrain);
        
        std::string name;
        
        float nexusScale;
        float nexusRadius;
        tiny::vec4 nexusPosition;
        tiny::draw::StaticMeshHorde *nexusMesh;
        tiny::draw::RGBATexture2D *nexusDiffuseTexture;
        tiny::draw::RGBATexture2D *nexusNormalTexture;
        
        float towerScale;
        float towerRadius;
        std::list<tiny::vec4> towerPositions;
        tiny::draw::StaticMeshHorde *towerMeshes;
        tiny::draw::RGBATexture2D *towerDiffuseTexture;
        tiny::draw::RGBATexture2D *towerNormalTexture;
        
        std::list<MinionSpawner> minionSpawners;
};

} //namespace moba

