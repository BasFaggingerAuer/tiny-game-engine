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
#include <exception>

#include <tiny/net/host.h>

using namespace tiny::net;

Host::Host(const unsigned int &listenPort, MessageTranslator *a_translator) :
    lastClientIndex(0),
    translator(a_translator)
{
    if (!translator)
    {
        std::cerr << "Please supply a message translator!" << std::endl;
        throw std::exception();
    }
    
    //Create listening socket.
    if (SDLNet_ResolveHost(&hostAddress, 0, listenPort) < 0)
    {
        std::cerr << "Unable to listen at port " << listenPort << ": " << SDLNet_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    hostSocket = SDLNet_TCP_Open(&hostAddress);
    
    if (!hostSocket)
    {
        std::cerr << "Unable to create listening socket at port " << listenPort << ": " << SDLNet_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Start listening for clients.
    std::cerr << "Hosting at " << SDLNet_ResolveIP(&hostAddress) << ":" << listenPort << "." << std::endl;
}

Host::~Host()
{
    //Terminate connections.
    std::cerr << "Closing " << clients.size() << " external connections..." << std::endl;

    for (std::map<TCPsocket, unsigned int>::const_iterator c = clients.begin(); c != clients.end(); ++c)
    {
        SDLNet_TCP_Close(c->first);
    }

    SDLNet_TCP_Close(hostSocket);
    
    std::cerr << "Stopped hosting." << std::endl;
}

bool Host::listen(const double &dt)
{
    //Create socket selector for the current number of clients.
    SDLNet_SocketSet selector = SDLNet_AllocSocketSet(clients.size() + 2);
    
    if (!selector)
    {
        std::cerr << "Unable to create host selector: " << SDLNet_GetError() << "!" << std::endl;
        return false;
    }
    
    //Populate selector.
    if (SDLNet_TCP_AddSocket(selector, hostSocket) < 0)
    {
        std::cerr << "Unable to add listening socket to selector: " << SDLNet_GetError() << "!" << std::endl;
        return false;
    }
    
    for (std::map<TCPsocket, unsigned int>::const_iterator c = clients.begin(); c != clients.end(); ++c)
    {
        if (SDLNet_TCP_AddSocket(selector, c->first) < 0)
        {
            std::cerr << "Unable to add client " << c->second << "'s socket to selector: " << SDLNet_GetError() << "!" << std::endl;
            return false;
        }
    }
    
    //Are any of the sockets active?
    //TODO: Keep doing this until there is no more data available?
    if (SDLNet_CheckSockets(selector, 1000.0*dt) > 0)
    {
        if (SDLNet_SocketReady(hostSocket) != 0)
        {
            //New incoming connection.
            TCPsocket clientSocket = SDLNet_TCP_Accept(hostSocket);
            
            if (clientSocket)
            {
                IPaddress *clientAddress = SDLNet_TCP_GetPeerAddress(clientSocket);
                
                //SDLNet_TCP_AddSocket(selector, clientSocket);
                
                std::cerr << "Accepted connection from " << SDLNet_ResolveIP(clientAddress) << " with index " << lastClientIndex << "." << std::endl;
                
                clients.insert(std::make_pair(clientSocket, lastClientIndex));
                addClient(lastClientIndex);
                lastClientIndex++;
            }
            else
            {
                std::cerr << "Warning: Unable to accept incoming connection: " << SDLNet_GetError() << "!" << std::endl;
            }
        }
        
        //Look for data from the clients and remove the clients that have disconnected.
        std::map<TCPsocket, unsigned int> remainingClients;
        
        for (std::map<TCPsocket, unsigned int>::const_iterator c = clients.begin(); c != clients.end(); ++c)
        {
            bool keepClient = true;
            
            if (SDLNet_SocketReady(c->first) != 0)
            {
                //Receive data.
                Message message;
                
                if (!translator->receiveMessageTCP(c->first, message))
                {
                    std::cerr << "Lost connection from client " << c->second << "." << std::endl;
                    removeClient(c->second);
                    SDLNet_TCP_Close(c->first);
                    keepClient = false;
                }
                else
                {
                    receiveMessage(c->second, message);
                }
            }
            
            if (keepClient)
            {
                remainingClients.insert(*c);
            }
        }
        
        clients = remainingClients;
    }
    
    //Free socket set.
    SDLNet_FreeSocketSet(selector);
    
    return true;
}

void Host::sendMessage(const Message &message)
{
    //Send message to all clients.
    for (std::map<TCPsocket, unsigned int>::const_iterator c = clients.begin(); c != clients.end(); ++c)
    {
        if (!translator->sendMessageTCP(message, c->first))
        {
            std::cerr << "Unable to send message " << message.id << " to client " << c->second << "!" << std::endl;
        }
    }
}

void Host::sendPrivateMessage(const Message &message, const unsigned int &clientIndex)
{
    //Send message to a specific client.
    for (std::map<TCPsocket, unsigned int>::const_iterator c = clients.begin(); c != clients.end(); ++c)
    {
        if (c->second == clientIndex)
        {
            if (!translator->sendMessageTCP(message, c->first))
            {
                std::cerr << "Unable to send private message " << message.id << " to client " << c->second << "!" << std::endl;
            }
            
            return;
        }
    }
    
    std::cerr << "Unable to find client with index " << clientIndex << "!" << std::endl;
}

void Host::addClient(const unsigned int &)
{

}

void Host::receiveMessage(const unsigned int &, const Message &)
{

}

void Host::removeClient(const unsigned int &)
{

}

void Host::kickClient(const unsigned int &)
{
    //TODO.
}

