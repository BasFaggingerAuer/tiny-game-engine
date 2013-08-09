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
#include <map>
#include <string>
#include <exception>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include <tiny/net/message.h>
#include <tiny/net/console.h>
#include <tiny/net/client.h>
#include <tiny/net/host.h>

#include <tiny/img/io/image.h>

#include <tiny/draw/renderer.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>

#include "messages.h"

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

using namespace tanks;

TanksClient::TanksClient(const std::string &hostName, const unsigned int &hostPort, TanksGame *a_game) :
    tiny::net::Client(hostName, hostPort, a_game->getTranslator()),
    game(a_game)
{

}

TanksClient::~TanksClient()
{

}

void TanksClient::receiveMessage(const Message &message)
{
    game->applyMessage(0, message);
}

void TanksClient::disconnectedFromHost()
{
    game->disconnect();
}

TanksHost::TanksHost(const unsigned int &hostPort, TanksGame *a_game) :
    tiny::net::Host(hostPort, a_game->getTranslator()),
    game(a_game)
{

}

TanksHost::~TanksHost()
{

}

void TanksHost::addClient(const unsigned int &clientIndex)
{
    game->addPlayer(clientIndex);
}

void TanksHost::receiveMessage(const unsigned int &clientIndex, const Message &message)
{
    game->applyMessage(clientIndex, message);
}

void TanksHost::removeClient(const unsigned int &clientIndex)
{
    game->removePlayer(clientIndex);
}

TanksConsole::TanksConsole(TanksGame *a_game) :
    game(a_game)
{

}

TanksConsole::~TanksConsole()
{

}

void TanksConsole::execute(const std::string &command)
{
    Message message;
    
    if (command.empty()) return;
    
    if (!game->getTranslator()->textToMessage(command, message))
    {
        addLine("Unknown command or parameters!");
    }
    else if (!game->applyMessage(0, message))
    {
        addLine("Invalid command!");
    }
}

Player::Player()
{

}

Player::~Player()
{

}

TanksGame::TanksGame(const std::string &a_resourcePath) :
    resourcePath(a_resourcePath),
    translator(new TanksMessageTranslator()),
    console(new TanksConsole(this)),
    host(0),
    client(0)
{
    std::cerr << "Starting a game of tanks with resources from '" << resourcePath << "'..." << std::endl;
    
    //Read font from disk as a texture.
    fontTexture = new tiny::draw::IconTexture2D(512, 512);
    fontTexture->packIcons(tiny::img::io::readFont(resourcePath + "font/OpenBaskerville-0.0.75.ttf", 48));
    
    //Create a drawable font object as a collection of instanced screen icons.
    font = new tiny::draw::ScreenIconHorde(1024);
    font->setIconTexture(*fontTexture);
    font->setText(-1.0, -1.0, 0.2, 2.0, "Welcome to \\ra Game of Tanks\\w, press <ENTER> to start.", *fontTexture);
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new tiny::draw::Renderer();
    renderer->addRenderable(font, false, false, tiny::draw::BlendMix);
}

TanksGame::~TanksGame()
{
    disconnect();
    
    delete translator;
}

void TanksGame::update(tiny::os::Application *application, const double &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Update console.
    bool updateConsole = false;
    
    for (int i = 0; i < 256; ++i)
    {
        if (application->isKeyPressedOnce(i))
        {
            updateConsole = true;
            console->keyDown(i);
        }
    }
    
    if (updateConsole)
    {
        font->setText(-1.0, -1.0, 0.1, static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight()), console->getText(32), *fontTexture);
    }
}

void TanksGame::render()
{
    renderer->clearTargets();
    renderer->render();
}

bool TanksGame::applyMessage(const unsigned int &playerIndex, const Message &message)
{
    std::ostringstream out;
    bool ok = true;
    
    if (message.id == msg::mt::none)
    {
        out << "None message should never be sent.";
        ok = false;
    }
    else if (message.id == msg::mt::help)
    {
        if (playerIndex == 0)
        {
            out << "Available commands: " << translator->getMessageTypeNames();
        }
    }
    else if (message.id == msg::mt::host)
    {
        if (!host && !client)
        {
            try
            {
                unsigned int port = message.data[0].iv1;
                
                //Host on port 1234 if nothing is specified.
                if (port == 0)
                {
                    port = 1234;
                }
                
                host = new TanksHost(port, this);
            }
            catch (std::exception &e)
            {
                out << "Unable to host game!";
                ok = false;
            }
        }
        else
        {
            out << "A network game is already in progress, please disconnect first.";
            ok = false;
        }
    }
    else if (message.id == msg::mt::join)
    {
        if (!host && !client)
        {
            try
            {
                std::ostringstream ipAddress;
                unsigned int port = message.data[2].iv1;
                
                //Join localhost:1234 if no host is specified.
                if (port == 0)
                {
                    port = 1234;
                }
                
                if (message.data[0].iv1 == 0 && message.data[1].iv1 == 0)
                {
                    ipAddress << "localhost";
                }
                else
                {
                    //Extract IP address from two integers.
                    ipAddress << ((message.data[0].iv1/1000) % 1000) << "." << (message.data[0].iv1 % 1000) << "."
                              << ((message.data[1].iv1/1000) % 1000) << "." << (message.data[1].iv1 % 1000);
                }
                
                client = new TanksClient(ipAddress.str(), port, this);
            }
            catch (std::exception &e)
            {
                out << "Unable to join game!";
                ok = false;
            }
        }
        else
        {
            out << "A network game is already in progress, please disconnect first.";
            ok = false;
        }
    }
    else
    {
        out << "Message has unknown identifier " << message.id << "!";
        ok = false;
    }
    
    if (!out.str().empty()) console->addLine(out.str());
    
    return ok;
}

TanksMessageTranslator *TanksGame::getTranslator() const
{
    return translator;
}

void TanksGame::disconnect()
{
    if (client)
    {
        delete client;
        
        client = 0;
    }
    
    if (host)
    {
        delete host;
        
        host = 0;
    }
    
    console->addLine("Disconnected from network.");
}

void TanksGame::addPlayer(const unsigned int &playerIndex)
{
    std::ostringstream out;
    
    if (players.find(playerIndex) != players.end())
    {
        out << "Player with index " << playerIndex << " will not be added, this player already exists!";
    }
    else
    {
        if (host)
        {
            //Send this player to all other clients.
            if (true)
            {
                Message msg(msg::mt::addPlayer);
                
                msg << static_cast<int>(playerIndex);
                host->sendMessage(msg);
            }
            
            //Send all current players to the new client.
            for (std::map<unsigned int, Player>::const_iterator j = players.begin(); j != players.end(); ++j)
            {
                Message msg(msg::mt::addPlayer);
                
                msg << static_cast<int>(j->first);
                host->sendPrivateMessage(msg, playerIndex);
            }
        }
    
        players.insert(std::make_pair(playerIndex, Player()));
        out << "Added player with index " << playerIndex << ".";
    }
    
    console->addLine(out.str());
}

void TanksGame::removePlayer(const unsigned int &playerIndex)
{
    std::ostringstream out;
    std::map<unsigned int, Player>::iterator i = players.find(playerIndex);
    
    if (i == players.end())
    {
        out << "Player with index " << playerIndex << " will not be removed, this player does not exist!";
    }
    else
    {
        if (host)
        {
            //Broadcast removal of this player.
            Message msg(msg::mt::removePlayer);
            
            msg << static_cast<int>(playerIndex);
            host->sendMessage(msg);
        }
    
        players.erase(i);
        out << "Removed player with index " << playerIndex << ".";
    }
    
    console->addLine(out.str());
}

int main(int argc, char **argv)
{
    tiny::os::Application *application = 0;
    TanksGame *game = 0;
    
    try
    {
        int screenWidth = 800;
        int screenHeight = 600;
        
        if (argc != 2 && argc != 4)
        {
            std::cerr << "Usage: " << argv[0] << " path/to/resources [screenwidth screenheight]" << std::endl;
            throw std::exception();
        }
        
        if (argc == 4)
        {
            screenWidth = atoi(argv[2]);
            screenHeight = atoi(argv[3]);
        }
        
        application = new tiny::os::SDLApplication(screenWidth, screenHeight);
        game = new TanksGame(argv[1]);
    }
    catch (std::exception &e)
    {
        std::cerr << "Unable to start application!" << std::endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        game->update(application, application->pollEvents());
        game->render();
        application->paint();
    }
    
    delete game;
    delete application;
    
    std::cerr << "Goodbye." << std::endl;
    
    return 0;
}

