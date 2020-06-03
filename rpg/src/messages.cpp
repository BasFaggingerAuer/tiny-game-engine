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
#include "messages.h"

namespace rpg
{

namespace msg
{

class Help : public tiny::net::MessageType
{
    public:
        Help() : tiny::net::MessageType(mt::help, "help", "Display help.")
        {
            
        }
        
        ~Help() {}
};

class Host : public tiny::net::MessageType
{
    public:
        Host() : tiny::net::MessageType(mt::host, "host", "Start hosting a game at a desired port.")
        {
            addVariableType("port", tiny::net::vt::Integer);
        }
        
        ~Host() {}
};

class Join : public tiny::net::MessageType
{
    public:
        Join() : tiny::net::MessageType(mt::join, "join", "Join a game at a specific ip address (192.168.1.4 becomes 192168 and 001004) and port.")
        {
            addVariableType("ip1", tiny::net::vt::Integer);
            addVariableType("ip2", tiny::net::vt::Integer);
            addVariableType("port", tiny::net::vt::Integer);
        }
        
        ~Join() {}
};

class Disconnect : public tiny::net::MessageType
{
    public:
        Disconnect() : tiny::net::MessageType(mt::disconnect, "disconnect", "Disconnect the current network game.")
        {
            
        }
        
        ~Disconnect() {}
};

class AddPlayer : public tiny::net::MessageType
{
    public:
        AddPlayer() : tiny::net::MessageType(mt::addPlayer, "addplayer", "Adds a human player to the game, with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~AddPlayer() {}
};

class RemovePlayer : public tiny::net::MessageType
{
    public:
        RemovePlayer() : tiny::net::MessageType(mt::removePlayer, "removeplayer", "Removes a human player to the game, with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~RemovePlayer() {}
};

class WelcomePlayer : public tiny::net::MessageType
{
    public:
        WelcomePlayer() : tiny::net::MessageType(mt::welcomePlayer, "welcomeplayer", "Sends the index of a player to its client (host -> client only).")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~WelcomePlayer() {}
};

class TerrainOffset : public tiny::net::MessageType
{
    public:
        TerrainOffset() : tiny::net::MessageType(mt::terrainOffset, "terrainoffset", "Shifts the action to a different part of the terrain.")
        {
            addVariableType("offset", tiny::net::vt::Vec2);
        }
        
        ~TerrainOffset() {}
};

class AddCharacter : public tiny::net::MessageType
{
    public:
        AddCharacter() : tiny::net::MessageType(mt::addCharacter, "addcharacter", "Creates a character with given index of a given type at a specific spot on the terrain, without a controller.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("type", tiny::net::vt::Integer);
            addVariableType("color", tiny::net::vt::Float);
        }
        
        ~AddCharacter() {}
};

class RemoveCharacter : public tiny::net::MessageType
{
    public:
        RemoveCharacter() : tiny::net::MessageType(mt::removeCharacter, "removecharacter", "Removes a character with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~RemoveCharacter() {}
};

class UpdateCharacter : public tiny::net::MessageType
{
    public:
        UpdateCharacter() : tiny::net::MessageType(mt::updateCharacter, "updatecharacter", "Sets the parameters of a character with a specific index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("pos", tiny::net::vt::Vec3);
            addVariableType("rot", tiny::net::vt::Float);
            addVariableType("color", tiny::net::vt::Float);
        }
        
        ~UpdateCharacter() {}
};

class SetPlayerCharacter : public tiny::net::MessageType
{
    public:
        SetPlayerCharacter() : tiny::net::MessageType(mt::setPlayerCharacter, "setplayercharacter", "Sets the controller of a character to be a certain player.")
        {
            addVariableType("player", tiny::net::vt::Integer);
            addVariableType("character", tiny::net::vt::Integer);
        }
        
        ~SetPlayerCharacter() {}
};

class PlayerSpawnRequest : public tiny::net::MessageType
{
    public:
        PlayerSpawnRequest() : tiny::net::MessageType(mt::playerSpawnRequest, "spawn", "A player requests to spawn in a certain character type.")
        {
            addVariableType("type", tiny::net::vt::Integer);
        }
        
        ~PlayerSpawnRequest() {}
};

} //namespace msg

} //namespace rpg

using namespace rpg;

GameMessageTranslator::GameMessageTranslator() :
    tiny::net::MessageTranslator()
{
    addMessageType(new msg::Help());
    addMessageType(new msg::Host());
    addMessageType(new msg::Join());
    addMessageType(new msg::Disconnect());
    addMessageType(new msg::AddPlayer());
    addMessageType(new msg::RemovePlayer());
    addMessageType(new msg::WelcomePlayer());
    addMessageType(new msg::TerrainOffset());
    addMessageType(new msg::AddCharacter());
    addMessageType(new msg::RemoveCharacter());
    addMessageType(new msg::UpdateCharacter());
    addMessageType(new msg::SetPlayerCharacter());
    addMessageType(new msg::PlayerSpawnRequest());
}

GameMessageTranslator::~GameMessageTranslator()
{

}

