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
#include <map>

#include <SDL_net.h>

#include <tiny/net/message.h>

namespace tiny
{

namespace net
{

class Host
{
    public:
        Host(const unsigned int &);
        virtual ~Host();
        
        bool listen(const double &);
        void sendMessage(const Message &);
        void sendPrivateMessage(const unsigned int &, const Message &);
        
    protected:
        void addMessageType(const MessageType *);
        
        virtual void addClient(const unsigned int &);
        virtual void receiveMessage(const unsigned int &, const Message &);
        virtual void removeClient(const unsigned int &);
        
        void kickClient(const unsigned int &);
        
    private:
        unsigned int lastClientIndex;
        IPaddress hostAddress;
        TCPsocket hostSocket;
        std::map<TCPsocket, unsigned int> clients;
        std::vector<const MessageType *> messageTypes;
        std::vector<unsigned char> messageBuffer;
};

}

}

