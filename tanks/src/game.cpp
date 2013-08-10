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
#include <exception>

#include "messages.h"
#include "game.h"

using namespace tanks;

Player::Player()
{

}

Player::~Player()
{

}

TanksGame::TanksGame(const std::string &a_resourcePath) :
    resourcePath(a_resourcePath),
    aspectRatio(1920.0/1080.0),
    translator(new TanksMessageTranslator()),
    console(new TanksConsole(this)),
    host(0),
    client(0),
    clientPlayerIndex(0)
{
    std::cerr << "Starting a game of tanks with resources from '" << resourcePath << "'..." << std::endl;
    
    //Read font from disk as a texture.
    fontTexture = new tiny::draw::IconTexture2D(512, 512);
    fontTexture->packIcons(tiny::img::io::readFont(resourcePath + "font/mensch.ttf", 48));
    
    //Create a drawable font object as a collection of instanced screen icons.
    font = new tiny::draw::ScreenIconHorde(1024);
    font->setIconTexture(*fontTexture);
    font->setText(-1.0, -1.0, 0.1, aspectRatio, "Welcome to \\ra Game of Tanks\\w, press <ENTER> to start.", *fontTexture);
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new tiny::draw::Renderer();
    renderer->addRenderable(font, false, false, tiny::draw::BlendMix);
}

TanksGame::~TanksGame()
{
    disconnect();
    
    delete translator;
}

void TanksGame::updateConsole() const
{
    //Update the console on screen.
    font->setText(-1.0, -1.0, 0.05, aspectRatio, console->getText(64), *fontTexture);
}

void TanksGame::update(tiny::os::Application *application, const double &dt)
{
    //Update aspect ratio.
    aspectRatio = static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight());
    
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Update console.
    for (int i = 0; i < 256; ++i)
    {
        if (application->isKeyPressedOnce(i))
        {
            console->keyDown(i);
        }
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
            out << "\\w\\4==== Available commands:" << std::endl << translator->getMessageTypeNames("\n") << std::endl;
            out << "\\w\\4==== Full descriptions:" << std::endl << translator->getMessageTypeDescriptions();
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
                out << "Hosting game at port " << port << ".";
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
                out << "Joined game at " << ipAddress.str() << ":" << port << ".";
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
    else if (message.id == msg::mt::disconnect)
    {
        if (playerIndex == 0)
        {
            out << "Disconnecting...";
            disconnect();
        }
        else
        {
            out << "Player " << playerIndex << " tried to force us to disconnect!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::addPlayer)
    {
        if (playerIndex == 0 && client)
        {
            addPlayer(message.data[0].iv1);
        }
        else
        {
            out << "Only clients add new players via network messages!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::removePlayer)
    {
        if (playerIndex == 0 && client)
        {
            removePlayer(message.data[0].iv1);
        }
        else
        {
            out << "Only clients remove players via network messages!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::welcomePlayer)
    {
        if (playerIndex == 0 && client)
        {
            clientPlayerIndex = message.data[0].iv1;
            out << "Welcomed to network game as player #" << clientPlayerIndex << ".";
        }
        else
        {
            out << "Only clients receive welcome messages!";
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
        console->addLine("Disconnected client from network.");
    }
    
    if (host)
    {
        delete host;
        
        host = 0;
        console->addLine("Disconnected host from network.");
    }
    
    //Remove all players.
    players.clear();
    clientPlayerIndex = 0;
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
            
            //Send new client a welcome message.
            if (true)
            {
                Message msg(msg::mt::welcomePlayer);
                
                msg << static_cast<int>(playerIndex);
                host->sendPrivateMessage(msg, playerIndex);
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

