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

#include <tiny/net/console.h>
#include <tiny/net/client.h>
#include <tiny/net/host.h>

namespace tanks
{

typedef tiny::net::Message Message;

class TanksGame;

class TanksConsole : public tiny::net::Console
{
    public:
        TanksConsole(TanksGame *);
        ~TanksConsole();
    
    protected:
        void execute(const std::string &);
        
    private:
        TanksGame * const game;
};

class TanksHost : public tiny::net::Host
{
    public:
        TanksHost(const unsigned int &, TanksGame *);
        ~TanksHost();
        
    protected:
        void addClient(const unsigned int &);
        void receiveMessage(const unsigned int &, const tiny::net::Message &);
        void removeClient(const unsigned int &);
        
    private:
        TanksGame * const game;
};

class TanksClient : public tiny::net::Client
{
    public:
        TanksClient(const std::string &, const unsigned int &, TanksGame *);
        ~TanksClient();
        
    protected:
        void receiveMessage(const tiny::net::Message &);
        void disconnectedFromHost();
        
    private:
        TanksGame * const game;
};

} //namespace tanks

