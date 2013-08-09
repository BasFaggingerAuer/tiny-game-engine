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

#include <tiny/net/message.h>

namespace tanks
{

namespace msg
{

namespace mt
{

enum mt_t
{
    none = 0,
    help,
    host,
    join,
    addPlayer,
    removePlayer,
    welcomePlayer,
    addTank,
    removeTank,
    updateTank,
    setTankControls,
    setPlayerTank
};

} //namespace mt

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

class AddPlayer : public tiny::net::MessageType
{
    public:
        AddPlayer() : tiny::net::MessageType(mt::addPlayer, "addPlayer", "Adds a human player to the game, with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~AddPlayer() {}
};

class RemovePlayer : public tiny::net::MessageType
{
    public:
        RemovePlayer() : tiny::net::MessageType(mt::removePlayer, "removePlayer", "Removes a human player to the game, with the given index.")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~RemovePlayer() {}
};

class WelcomePlayer : public tiny::net::MessageType
{
    public:
        WelcomePlayer() : tiny::net::MessageType(mt::welcomePlayer, "welcomePlayer", "Sends the index of a player to its client (host -> client only).")
        {
            addVariableType("index", tiny::net::vt::Integer);
        }
        
        ~WelcomePlayer() {}
};

} //namespace msg

class TanksMessageTranslator : public tiny::net::MessageTranslator
{
    public:
        TanksMessageTranslator();
        ~TanksMessageTranslator();
};

} //namespace tanks

