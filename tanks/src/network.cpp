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

#include "messages.h"
#include "network.h"
#include "game.h"

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
    game->clear();
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
    Message msg(msg::mt::addPlayer);
    
    msg << clientIndex;
    
    game->applyMessage(0, msg);
}

void TanksHost::receiveMessage(const unsigned int &clientIndex, const Message &message)
{
    game->applyMessage(clientIndex, message);
}

void TanksHost::removeClient(const unsigned int &clientIndex)
{
    Message msg(msg::mt::removePlayer);
    
    msg << clientIndex;
    
    game->applyMessage(0, msg);
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
    
    if (command.empty())
    {
        return;
    }
    
    if (!game->getTranslator()->textToMessage(command, message))
    {
        addLine("Unknown command or parameters!");
        return;
    }
    
    if (!game->userMessage(message))
    {
        addLine("Invalid command!");
    }
}

void TanksConsole::update()
{
    game->updateConsole();
}


