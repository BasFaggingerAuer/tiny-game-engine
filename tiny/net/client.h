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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <SDL_net.h>

#include <tiny/net/message.h>

namespace tiny
{

namespace net
{

class Client : public MessageTranslator
{
    public:
        Client(const std::string &, const unsigned int &);
        virtual ~Client();
        
        bool listen(const double &);
        void sendMessage(const Message &);
        
    protected:
        virtual void receiveMessage(const Message &);
        virtual void disconnectedFromHost();
        
    private:
        IPaddress hostAddress;
        TCPsocket socket;
};

}

}

