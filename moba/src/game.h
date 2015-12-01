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
#include <tiny/draw/effects/solid.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/snd/source.h>

#include "terrain.h"
#include "forest.h"
#include "faction.h"
#include "minion_type.h"

namespace moba
{

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
        
        //Renderer.
        const double aspectRatio;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        //Sky and atmosphere.
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSky *skyEffect;
        
        GameTerrain *terrain;
        GameForest *forest;
        std::map<std::string, MinionType *> minionTypes;
        std::map<unsigned int, Minion> moba;
        std::vector<Faction *> factions;
};

} //namespace moba

