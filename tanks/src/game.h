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

#include <tinyxml.h>

#include <tiny/os/application.h>
#include <tiny/img/io/image.h>

#include <tiny/draw/renderer.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/terrain.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/texture2darray.h>
#include <tiny/draw/computetexture.h>
#include <tiny/draw/heightmap/scale.h>
#include <tiny/draw/heightmap/resize.h>
#include <tiny/draw/heightmap/normalmap.h>
#include <tiny/draw/heightmap/tangentmap.h>
#include <tiny/draw/heightmap/diamondsquare.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/worldrenderer.h>

#include "network.h"

namespace tanks
{

class Player
{
    public:
        Player();
        ~Player();
};

class TanksGame
{
    public:
        TanksGame(const tiny::os::Application *, const std::string &);
        ~TanksGame();
        
        void update(tiny::os::Application *, const double &);
        void render();
        
        TanksMessageTranslator *getTranslator() const;
        bool applyMessage(const unsigned int &, const Message &);
        void disconnect();
        void addPlayer(const unsigned int &);
        void removePlayer(const unsigned int &);
        void updateConsole() const;
        
    private:
        void readResources(const std::string &);
        void readConsoleResources(const std::string &, TiXmlElement *);
        void readSkyResources(const std::string &, TiXmlElement *);
        void readTerrainResources(const std::string &, TiXmlElement *);
        
        void setTerrainOffset(const tiny::vec2 &);
        
        const double aspectRatio;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        bool consoleMode;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::IconTexture2D *fontTexture;
        
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSky *skyEffect;
        
        tiny::draw::Terrain *terrain;
        tiny::vec2 terrainScale;
        tiny::ivec2 terrainFarScale;
        tiny::vec2 terrainDetailScale;
        tiny::vec2 terrainFarOffset;
        tiny::draw::FloatTexture2D *terrainHeightTexture;
        tiny::draw::FloatTexture2D *terrainFarHeightTexture;
        tiny::draw::RGBTexture2D *terrainNormalTexture;
        tiny::draw::RGBTexture2D *terrainFarNormalTexture;
        tiny::draw::RGBTexture2D *terrainTangentTexture;
        tiny::draw::RGBTexture2D *terrainFarTangentTexture;
        tiny::draw::RGBATexture2D *terrainAttributeTexture;
        tiny::draw::RGBATexture2D *terrainFarAttributeTexture;
        
        tiny::draw::RGBTexture2DArray *biomeDiffuseTextures;
        tiny::draw::RGBTexture2DArray *biomeNormalTextures;
        
        TanksMessageTranslator * const translator;
        TanksConsole * const console;
        TanksHost *host;
        TanksClient *client;
        unsigned int clientPlayerIndex;
        
        std::map<unsigned int, Player> players;
};

} //namespace tanks

