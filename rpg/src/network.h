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

#include <tiny/net/console.h>
#include <tiny/net/client.h>
#include <tiny/net/host.h>

namespace rpg
{

typedef tiny::net::Message Message;

class Game;

class GameConsole : public tiny::net::Console
{
    public:
        GameConsole(Game *);
        ~GameConsole();
    
    protected:
        void execute(const std::string &);
        void update();
        
    private:
        Game * const game;
};

class GameHost : public tiny::net::Host
{
    public:
        GameHost(const unsigned int &, Game *);
        ~GameHost();
        
    protected:
        void addClient(const unsigned int &);
        void receiveMessage(const unsigned int &, const tiny::net::Message &);
        void removeClient(const unsigned int &);
        
    private:
        Game * const game;
};

class GameClient : public tiny::net::Client
{
    public:
        GameClient(const std::string &, const unsigned int &, Game *);
        ~GameClient();
        
    protected:
        void receiveMessage(const tiny::net::Message &);
        void disconnectedFromHost();
        
    private:
        Game * const game;
};

} //namespace rpg

