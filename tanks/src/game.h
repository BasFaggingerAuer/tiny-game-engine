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

#include <tiny/os/application.h>
#include <tiny/img/io/image.h>
#include <tiny/draw/renderer.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>

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
        TanksGame(const std::string &);
        ~TanksGame();
        
        void update(tiny::os::Application *, const double &);
        void render();
        
        TanksMessageTranslator *getTranslator() const;
        bool applyMessage(const unsigned int &, const Message &);
        void disconnect();
        void addPlayer(const unsigned int &);
        void removePlayer(const unsigned int &);
        
    private:
        const std::string resourcePath;
        tiny::draw::Renderer *renderer;
        tiny::draw::ScreenIconHorde *font;
        tiny::draw::IconTexture2D *fontTexture;
        
        TanksMessageTranslator * const translator;
        TanksConsole * const console;
        TanksHost *host;
        TanksClient *client;
        
        std::map<unsigned int, Player> players;
};

} //namespace tanks

