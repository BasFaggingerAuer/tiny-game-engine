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
#include <vector>
#include <string>
#include <exception>

#include <config.h>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>
#include <tiny/net/message.h>
#include <tiny/net/host.h>
#include <tiny/net/client.h>

class DisplayNumberMessageType : public tiny::net::MessageType
{
    public:
        DisplayNumberMessageType() :
            tiny::net::MessageType(1, "display", "Writes the supplied number to the standard output.")
        {
            addVariableType("number", tiny::net::vt::Integer);
        }

        ~DisplayNumberMessageType()
        {

        }
};

class SquareNumberMessageType : public tiny::net::MessageType
{
    public:
        SquareNumberMessageType() :
            tiny::net::MessageType(2, "square", "Requests the supplied number to be squared.")
        {
            addVariableType("number", tiny::net::vt::Integer);
        }
        
        ~SquareNumberMessageType()
        {

        }
};

class SimpleTranslator : public tiny::net::MessageTranslator
{
    public:
        SimpleTranslator() :
            tiny::net::MessageTranslator()
        {
            addMessageType(new DisplayNumberMessageType());
            addMessageType(new SquareNumberMessageType());
        }
        
        ~SimpleTranslator()
        {

        }
};

class SimpleClient : public tiny::net::Client
{
    public:
        SimpleClient(const std::string &hostName, tiny::net::MessageTranslator *a_translator) :
            tiny::net::Client(hostName, 1234, a_translator),
            isConnected(true)
        {

        }
        
        ~SimpleClient()
        {

        }
        
        void receiveMessage(const tiny::net::Message &message)
        {
            if (message.id == 1)
            {
                std::cout << "Received integer printing request: " << message.data[0].iv1 << "." << std::endl;
            }
            else if (message.id == 2)
            {
                std::cout << "Received integer squaring request, which is silly, that is the host's job!" << std::endl;
                std::cout << "Received integer squaring request for " << message.data[0].iv1 << "." << std::endl;
                
                tiny::net::Message returnMessage(1);
                
                returnMessage << message.data[0].iv1*message.data[0].iv1;
                sendMessage(returnMessage);
            }
            else
            {
                std::cerr << "Unknown message type!" << std::endl;
            }
        }
        
        void disconnectedFromHost()
        {
            std::cout << "The connection has been terminated!" << std::endl;
            isConnected = false;
        }
        
        bool isConnected;
};

class SimpleHost : public tiny::net::Host
{
    public:
        SimpleHost(tiny::net::MessageTranslator *a_translator) :
            tiny::net::Host(1234, a_translator)
        {

        }
        
        ~SimpleHost()
        {

        }
        
        void receiveMessage(const unsigned int &clientIndex, const tiny::net::Message &message)
        {
            if (message.id == 1)
            {
                std::cout << "Received integer printing request, which the host will perform reluctantly, " << message.data[0].iv1 << "." << std::endl;
            }
            else if (message.id == 2)
            {
                std::cout << "Received integer squaring request for " << message.data[0].iv1 << "." << std::endl;
                
                tiny::net::Message returnMessage(1);
                
                returnMessage << message.data[0].iv1*message.data[0].iv1;
                sendPrivateMessage(returnMessage, clientIndex);
            }
            else
            {
                std::cerr << "Unknown message type!" << std::endl;
            }
        }
        
        void addClient(const unsigned int &clientIndex)
        {
            std::cout << "Welcome, new client #" << clientIndex << "!" << std::endl;
        }
        
        void removeClient(const unsigned int &clientIndex)
        {
            std::cout << "Client #" << clientIndex << " is regrettably no longer with us." << std::endl;
        }
};

tiny::os::Application *application = 0;
SimpleClient *client = 0;
SimpleHost *host = 0;
SimpleTranslator *translator = 0;

using namespace std;

void cleanup()
{
    if (host) delete host;
    if (client) delete client;
    delete translator;
}

void render()
{
    if (host)
    {
        host->listen(0.5);
        
        if ((rand() & 255) == 0)
        {
            std::cout << "Sending number printing exercise to clients..." << std::endl;
            
            tiny::net::Message message(1);
            
            message << (rand() & 255);
            
            host->sendMessage(message);
        }
        else
        {
            std::cout << "Twiddling thumbs..." << std::endl;
        }
    }
    
    if (client)
    {
        client->listen(0.5);
        
        if ((rand() & 3) == 0)
        {
            const int number = rand() & 7;
            
            std::cout << "Sending number " << number << " to be squared to host..." << std::endl;
            
            tiny::net::Message message(2);
            
            message << number;
            client->sendMessage(message);
        }
        else
        {
            std::cout << "Twiddling thumbs..." << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    try
    {
        application = new tiny::os::SDLApplication(32, 32);
        
        translator = new SimpleTranslator();
        
        if (argc == 1)
        {
            host = new SimpleHost(translator);
        }
        else if (argc == 2)
        {
            client = new SimpleClient(argv[1], translator);
        }
        else
        {
            cerr << "Provide either zero or one command line arguments to act either as host or as client!" << std::endl;
            throw std::exception();
        }
        
        std::cout << translator->getMessageTypeNames("\n") << std::endl << translator->getMessageTypeDescriptions();
    }
    catch (std::exception &e)
    {
        cerr << "Unable to start application!" << endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        application->pollEvents();
        render();
        application->paint();
        
        if (client)
        {
            if (!client->isConnected) break;
        }
    }
    
    cleanup();
    delete application;
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

