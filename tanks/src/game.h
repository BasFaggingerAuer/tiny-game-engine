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

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/effects/solid.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/snd/source.h>

#include "network.h"
#include "terrain.h"
#include "soldier.h"

namespace tanks
{

class Player
{
    public:
        Player();
        ~Player();
        
        unsigned int soldierIndex;
};

class Game
{
    public:
        Game(const tiny::os::Application *, const std::string &);
        ~Game();
        
        void update(tiny::os::Application *, const float &);
        void render();
        
        GameMessageTranslator *getTranslator() const;
        bool applyMessage(const unsigned int &, const Message &);
        bool userMessage(const Message &);
        void clear();
        void updateConsole() const;
        
    private:
        bool msgHelp(const unsigned int &, std::ostream &, bool &);
        bool msgHost(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgJoin(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const unsigned int &);
        bool msgDisconnect(const unsigned int &, std::ostream &, bool &);
        bool msgAddPlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgRemovePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgWelcomePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgTerrainOffset(const unsigned int &, std::ostream &, bool &, const tiny::vec2 &);
        bool msgAddSoldier(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const tiny::vec2 &);
        bool msgRemoveSoldier(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgUpdateSoldier(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const tiny::vec2 &, const tiny::vec3 &, const tiny::vec4 &, const tiny::vec3 &);
        bool msgSetPlayerSoldier(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &);
        bool msgPlayerSpawnRequest(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgPlayerShootRequest(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgAddBullet(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const unsigned int &, const tiny::vec3 &, const tiny::vec3 &, const tiny::vec3 &);
        bool msgAddExplosion(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const tiny::vec3 &);
        
        void readResources(const std::string &);
        void readConsoleResources(const std::string &, TiXmlElement *);
        void readSkyResources(const std::string &, TiXmlElement *);
        void readBulletHordeResources(const std::string &, TiXmlElement *);
        
        void applyConsequences();
        
        //Renderer.
        const float aspectRatio;
        const float mouseSensitivity;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        //Console and font.
        bool consoleMode;
        tiny::draw::effects::Solid *consoleBackground;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::IconTexture2D *fontTexture;
        
        //Sky and atmosphere.
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSky *skyEffect;
        
        GameTerrain *terrain;
        
        float gravitationalConstant;
        
        //Sound sources.
        std::map<unsigned int, tiny::snd::Source *> soundSources;
        unsigned int lastSoundSourceIndex;
        
        //Soldiers.
        std::map<unsigned int, SoldierType *> soldierTypes;
        std::map<unsigned int, SoldierInstance> soldiers;
        unsigned int lastSoldierIndex;

        //Bullets.
        std::map<unsigned int, BulletType *> bulletTypes;
        std::map<unsigned int, BulletInstance> bullets;
        std::vector<tiny::draw::WorldIconInstance> bulletInstances;
        unsigned int lastBulletIndex;
        
        tiny::draw::IconTexture2D *bulletIconTexture;
        tiny::draw::WorldIconHorde *bulletHorde;
        
        //Explosions.
        std::map<unsigned int, ExplosionType *> explosionTypes;
        std::map<unsigned int, ExplosionInstance> explosions;
        unsigned int lastExplosionIndex;
        
        //Networking.
        GameMessageTranslator * const translator;
        GameConsole * const console;
        GameHost *host;
        GameClient *client;
        unsigned int ownPlayerIndex;
        
        std::map<unsigned int, Player> players;
};

} //namespace tanks

