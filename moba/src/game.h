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
#include <map>
#include <vector>

#include <tinyxml.h>

#include <tiny/os/application.h>

#include <tiny/mesh/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/mesh/animatedmesh.h>
#include <tiny/draw/animatedmeshhorde.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/effects/showimage.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/snd/source.h>

#include "terrain.h"
#include "forest.h"
#include "faction.h"
#include "minion_type.h"

namespace moba
{

class CollisionHashMap
{
    public:
        CollisionHashMap(const size_t &, const size_t &, const size_t &, const float &);
        ~CollisionHashMap();
        
        void buildCollisionBuckets(const std::vector<tiny::vec4> &);
        tiny::vec2 projectVelocity(const tiny::vec2 &, const float &, tiny::vec2) const;
    
    private:
        tiny::vec2 projectVelocityCylinder(const tiny::vec4 &, const tiny::vec2 &, const float &, tiny::vec2) const;
        
        const size_t nrBuckets;
        const size_t p1, p2;
        const float size;
        
        //Collision detection.
        std::vector<tiny::vec4> cylinders;
        std::vector<std::list<int> > buckets;
};

class Game
{
    public:
        Game(const tiny::os::Application *, const std::string &);
        ~Game();
        
        void clear();
        void update(tiny::os::Application *, const float &);
        void render();
        
    private:
        void readResources(const std::string &);
        void readSkyResources(const std::string &, TiXmlElement *);
        
        void spawnMinionAtPath(const std::string &, const std::string &, const std::string &, const float & = 0.0f);
        std::vector<tiny::vec4> createCollisionCylinders() const;
        
        //Renderer.
        const double aspectRatio;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        tiny::vec3 menuCameraPosition;
        tiny::vec4 menuCameraOrientation;
        tiny::draw::effects::ShowImage *logoLayer;
        tiny::draw::RGBATexture2D *logoTexture;
        tiny::draw::RGBATexture2D *giftTexture;
        
        tiny::vec3 spawnCameraPosition;
        tiny::vec4 spawnCameraOrientation;
        float spawnTime;
        float currentSpawnTime;
        int gameMode;
        
        //Sky and atmosphere.
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSky *skyEffect;
        
        GameTerrain *terrain;
        GameForest *forest;
        std::map<std::string, MinionType *> minionTypes;
        std::map<std::string, MinionPath *> minionPaths;
        std::map<unsigned int, Minion> minions;
        unsigned int minionIndex;
        std::map<std::string, Faction *> factions;
        
        std::list<tiny::vec4> staticCollisionCylinders;
        CollisionHashMap collisionHandler;
        
};

} //namespace moba

