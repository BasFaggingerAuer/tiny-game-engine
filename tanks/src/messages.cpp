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
#include "messages.h"

namespace tanks
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

class AddTank : public tiny::net::MessageType
{
    public:
        AddTank() : tiny::net::MessageType(mt::addTank, "addtank", "Creates a tank with given index of a given type at a specific spot on the terrain, without a controller.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("type", tiny::net::vt::Integer);
            addVariableType("pos", tiny::net::vt::Vec2);
        }
        
        ~AddTank() {}
};

class RemoveTank : public tiny::net::MessageType
{
    public:
        RemoveTank() : tiny::net::MessageType(mt::removeTank, "removetank", "Removes a tank with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~RemoveTank() {}
};

class UpdateTank : public tiny::net::MessageType
{
    public:
        UpdateTank() : tiny::net::MessageType(mt::updateTank, "updatetank", "Sets the parameters of a tank with a specific index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("pos", tiny::net::vt::Vec3);
            addVariableType("ori", tiny::net::vt::Vec4);
            addVariableType("vel", tiny::net::vt::Vec3);
            addVariableType("angvel", tiny::net::vt::Vec3);
        }
        
        ~UpdateTank() {}
};

class UpdateTankControls : public tiny::net::MessageType
{
    public:
        UpdateTankControls() : tiny::net::MessageType(mt::updateTankControls, "updatetankcontrols", "Sets the controls of a tank with a specific index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("controls", tiny::net::vt::Integer);
        }
        
        ~UpdateTankControls() {}
};

class UpdateTankController : public tiny::net::MessageType
{
    public:
        UpdateTankController() : tiny::net::MessageType(mt::updateTankController, "updatetankcontroller", "Sets the controller of a tank to be a bot or a player.")
        {
            addVariableType("index", tiny::net::vt::Integer);
            addVariableType("controller", tiny::net::vt::Integer);
        }
        
        ~UpdateTankController() {}
};

class PlayerSpawnRequest : public tiny::net::MessageType
{
    public:
        PlayerSpawnRequest() : tiny::net::MessageType(mt::playerSpawnRequest, "playerspawnrequest", "A player requests to spawn in a certain tank type.")
        {
            addVariableType("player", tiny::net::vt::Integer);
            addVariableType("type", tiny::net::vt::Integer);
        }
        
        ~PlayerSpawnRequest() {}
};

} //namespace msg

} //namespace tanks

using namespace tanks;

TanksMessageTranslator::TanksMessageTranslator() :
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
    addMessageType(new msg::AddTank());
    addMessageType(new msg::RemoveTank());
    addMessageType(new msg::UpdateTank());
    addMessageType(new msg::UpdateTankControls());
    addMessageType(new msg::UpdateTankController());
    addMessageType(new msg::PlayerSpawnRequest());
}

TanksMessageTranslator::~TanksMessageTranslator()
{

}

