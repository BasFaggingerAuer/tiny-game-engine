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

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/effects/solid.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/snd/source.h>

#include "network.h"
#include "terrain.h"

namespace rpg
{

class Player
{
    public:
        Player();
        ~Player();
        
        unsigned int characterIndex;
};

struct CharacterInstance
{
    CharacterInstance(const unsigned int &a_type = 0, const std::string &a_name = "", const tiny::vec3 &a_position = tiny::vec3(0.0f), const float &a_rotation = 0.0f, const float &a_color = 0.0f) :
        type(a_type),
        name(a_name),
        position(a_position),
        rotation(a_rotation),
        color(a_color)
    {
    
    }
    
    tiny::vec4 getColor() const
    {
        return (color >= 0.0f ? tiny::vec4(0.5f*(1.0f + cosf(2.0f*M_PI*(color + 0.0f/3.0f))),
                                0.5f*(1.0f + cosf(2.0f*M_PI*(color + 1.0f/3.0f))),
                                0.5f*(1.0f + cosf(2.0f*M_PI*(color + 2.0f/3.0f))),
                                1.0f) :
                                tiny::vec4(-color, -color, -color, 1.0f));
    }
    
    unsigned int type;
    std::string name;
    tiny::vec3 position;
    float rotation;
    float color;
};

class CharacterType
{
    public:
        CharacterType(const std::string &, TiXmlElement *);
        ~CharacterType();
        
        void clearInstances();
        void addInstance(const CharacterInstance &, const float &);
        void updateInstances();
        
        std::string name;
        tiny::vec3 size;
        
        tiny::draw::StaticMeshHorde *shadowHorde;
        tiny::draw::RGBTexture2D *shadowDiffuseTexture;
        tiny::draw::RGBTexture2D *shadowNormalTexture;
        
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        
        int nrInstances;
        int maxNrInstances;
        std::vector<tiny::draw::StaticMeshInstance> instances;
        std::vector<tiny::draw::StaticMeshInstance> shadowInstances;
};

class Chessboard
{
    public:
        Chessboard(const std::string &, TiXmlElement *);
        ~Chessboard();
        
        void updateInstances(const float &);
        
        float baseHeight;
        int nrSquares;
        std::list<tiny::ivec3> extraBlocks;
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
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
        bool msgAddCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &, const float &);
        bool msgRemoveCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        bool msgUpdateCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const tiny::vec3 &, const float &, const float &);
        bool msgSetPlayerCharacter(const unsigned int &, std::ostream &, bool &, const unsigned int &, const unsigned int &);
        bool msgPlayerSpawnRequest(const unsigned int &, std::ostream &, bool &, const unsigned int &);
        
        void readResources(const std::string &);
        void readCharacterResources(TiXmlElement *);
        void readConsoleResources(const std::string &, TiXmlElement *);
        void readSkyResources(const std::string &, TiXmlElement *);
        void readChessboardResources(const std::string &, TiXmlElement *);
        void readBulletHordeResources(const std::string &, TiXmlElement *);
        
        void applyConsequences();
        
        //Renderer.
        const double aspectRatio;
        const float mouseSensitivity;
        tiny::draw::WorldRenderer *renderer;
        
        tiny::vec3 cameraPosition;
        tiny::vec4 cameraOrientation;
        
        //Console and font.
        bool consoleMode;
        tiny::draw::effects::Solid *consoleBackground;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::WorldIconHorde *fontWorld;
        tiny::draw::IconTexture2D *fontTexture;
        
        //Chessboard.
        Chessboard *chessboard;
        
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
        
        //Characters.
        std::map<unsigned int, CharacterType *> characterTypes;
        std::map<unsigned int, CharacterInstance> characters;
        std::list<CharacterInstance> baseCharacters;
        unsigned int lastCharacterIndex;

        //Networking.
        GameMessageTranslator * const translator;
        GameConsole * const console;
        GameHost *host;
        GameClient *client;
        unsigned int ownPlayerIndex;
        
        std::map<unsigned int, Player> players;
};

} //namespace rpg

