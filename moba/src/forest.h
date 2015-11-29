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
#include <vector>

#include <tinyxml.h>

#include <tiny/lod/quadtree.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/iconhorde.h>

#include <tiny/draw/texture2d.h>

#include "terrain.h"

namespace moba
{

class GameForest
{
    public:
        GameForest(const std::string &, TiXmlElement *);
        ~GameForest();
        
        int plantTrees(const GameTerrain *terrain);
        void setCameraPosition(const tiny::vec3 &);
        
        tiny::draw::StaticMeshHorde *treeMeshes;
        tiny::draw::WorldIconHorde *treeSprites;
        
    private:
        int nrPlantedTrees;
        int maxNrHighDetailTrees;
        int maxNrLowDetailTrees;
        int biomeIndex;
        float treeHighDetailRadius;
        float treeLowDetailRadius;

        tiny::lod::Quadtree *quadtree;
        
        tiny::draw::RGBATexture2D *treeDiffuseTexture;
        tiny::draw::RGBATexture2D *treeSpriteTexture;
        tiny::vec2 treeSpriteSize;

        std::vector<tiny::draw::StaticMeshInstance> allTreeHighDetailInstances;
        std::vector<tiny::draw::WorldIconInstance> allTreeLowDetailInstances;

        std::vector<int> visibleTreeInstanceIndices;
        std::vector<tiny::draw::StaticMeshInstance> visibleTreeHighDetailInstances;
        std::vector<tiny::draw::WorldIconInstance> visibleTreeLowDetailInstances;
        
        std::vector<tiny::vec3> treePositions;
};

} //namespace moba

