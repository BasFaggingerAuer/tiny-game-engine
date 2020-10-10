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
#pragma once

#include <string>
#include <map>

#include <tinyxml.h>

#include <tiny/os/application.h>

#include <tiny/math/vec.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/voxelmap.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/texture3d.h>
#include <tiny/draw/texture2dcubearray.h>
#include <tiny/draw/effects/sunskyvoxelmap.h>
#include <tiny/draw/effects/solid.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/snd/source.h>

#include "network.h"
#include "terrain.h"
#include "voxel.h"
#include "character.h"

namespace rpg
{

class Player
{
    public:
        Player();
        ~Player();
        
        unsigned int characterIndex;
};

enum PaintMode
{
    VoxelReplace,
    VoxelAdd,
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
        bool msgJoin(const unsigned int &, std::ostream &, bool &, const std::string &, const unsigned int &);
        bool msgDisconnect(const unsigned int &, std::ostream &, bool &);
        bool msgAddPlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgRemovePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgWelcomePlayer(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgTerrainOffset(const unsigned int &, std::ostream &, bool &, const tiny::vec2 &);
        bool msgSunDirection(const unsigned int &, std::ostream &, bool &, const tiny::vec3 &);
        bool msgListCharacterTypes(const unsigned int &, std::ostream &, bool &);
        bool msgListCharacters(const unsigned int &, std::ostream &, bool &);
        bool msgAddCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const std::string &, const unsigned int &, const float &);
        bool msgRemoveCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgUpdateCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const tiny::ivec3 &, const int &, const int &, const float &);
        bool msgSetPlayerCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &);
        bool msgUpdateVoxel(const unsigned int &, std::ostream &, bool &, const tiny::ivec3 &, const unsigned int &);
        bool msgUpdateVoxelBasePlane(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgPlayerSpawnRequest(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        
        void readResources(const std::string &);
        void readCharacterResources(TiXmlElement *);
        void readConsoleResources(const std::string &, TiXmlElement *);
        void readSkyResources(const std::string &, TiXmlElement *);
        void readBulletHordeResources(const std::string &, TiXmlElement *);
        void readVoxelMapResources(const std::string &, TiXmlElement *);
        
        //Renderer.
        const double aspectRatio;
        const float mouseSensitivity;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        float mouseTimer;
        PaintMode paintMode;
        int paintVoxelType;
        
        //Console and font.
        bool consoleMode;
        tiny::draw::effects::Solid *consoleBackground;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::WorldIconHorde *fontWorld;
        tiny::draw::IconTexture2D *fontTexture;
        
        //Sky and atmosphere.
        tiny::draw::StaticMesh *skyBoxMesh;
        tiny::draw::RGBTexture2D *skyBoxDiffuseTexture;
        tiny::draw::RGBTexture2D *skyGradientTexture;
        tiny::draw::effects::SunSkyVoxelMap *skyEffect;
        
        //Terrain.
        GameTerrain *terrain;
        
        //Voxel map.
        GameVoxelMap *voxelMap;
        
        float gravitationalConstant;
        
        //Sound sources.
        std::map<unsigned int, tiny::snd::Source *> soundSources;
        unsigned int lastSoundSourceIndex;
        
        //Characters.
        std::map<unsigned int, CharacterType *> characterTypes;
        std::map<unsigned int, CharacterInstance> characters;
        unsigned int lastCharacterIndex;
        unsigned int selectedCharacterType;
        
        //Game starting state.
        std::list<std::string> initCommands;

        //Networking.
        GameMessageTranslator * const translator;
        GameConsole * const console;
        GameHost *host;
        GameClient *client;
        unsigned int ownPlayerIndex;
        
        std::map<unsigned int, Player> players;
};

} //namespace rpg

