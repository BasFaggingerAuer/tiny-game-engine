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
#include <iostream>

#include <tiny/net/client.h>

using namespace tiny::net;

Client::Client(const std::string &hostName, const unsigned int &hostPort, MessageTranslator *a_translator) :
    translator(a_translator)
{
    if (!translator)
    {
        std::cerr << "Please supply a message translator!" << std::endl;
        throw std::exception();
    }
    
    if (SDLNet_ResolveHost(&hostAddress, hostName.c_str(), hostPort) < 0)
    {
        std::cerr << "Unable to connect to host " << hostName << ":" << hostPort << ": " << SDLNet_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    socket = SDLNet_TCP_Open(&hostAddress);
    
    if (!socket)
    {
        std::cerr << "Unable to open socket to " << hostName << ":" << hostPort << ": " << SDLNet_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    std::cerr << "Connected to " << hostName << ":" << hostPort << "." << std::endl;
}

Client::~Client()
{
    std::cerr << "Closing connection..." << std::endl;
    
    SDLNet_TCP_Close(socket);
    
    std::cerr << "Stopped transmitting." << std::endl;
}

bool Client::listen(const double &dt)
{
    //Create socket selector.
    SDLNet_SocketSet selector = SDLNet_AllocSocketSet(3);
    
    if (!selector)
    {
        std::cerr << "Unable to create selector: " << SDLNet_GetError() << "!" << std::endl;
        return false;
    }
    
    //Populate selector.
    if (SDLNet_TCP_AddSocket(selector, socket) < 0)
    {
        std::cerr << "Unable to add socket to selector: " << SDLNet_GetError() << "!" << std::endl;
        return false;
    }
    
    //Are any of the sockets active?
    bool receivedData = true;
    
    while (receivedData)
    {
        receivedData = false;
        
        if (SDLNet_CheckSockets(selector, 1000.0*dt) > 0)
        {
            if (SDLNet_SocketReady(socket) != 0)
            {
                //Receive data.
                Message message;
                
                if (!translator->receiveMessageTCP(socket, message))
                {
                    std::cerr << "Lost connection to host!" << std::endl;
                    disconnectedFromHost();
                }
                else
                {
                    receiveMessage(message);
                    receivedData = true;
                }
            }
        }
    }
    
    //Free socket set.
    SDLNet_FreeSocketSet(selector);
    
    return true;
}

void Client::sendMessage(const Message &message)
{
    //Send message to host.
    if (!translator->sendMessageTCP(message, socket))
    {
        std::cerr << "Unable to send message " << message.id << " to host!" << std::endl;
    }
}

void Client::receiveMessage(const Message &)
{

}

void Client::disconnectedFromHost()
{
    std::cerr << "Disconnected from host." << std::endl;
}

