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
#include <fstream>
#include <vector>
#include <exception>

#include "messages.h"
#include "game.h"

using namespace tanks;
using namespace tiny;

bool TanksGame::msgHost(const unsigned int &, std::ostream &out, bool &, const unsigned int &port)
{
    if (!host && !client)
    {
        try
        {
            clear();
            host = new TanksHost(port, this);
            out << "Hosting game at port " << port << ".";
            
            //Enter the game as own player.
            ownPlayerIndex = 0;
            players.insert(std::make_pair(ownPlayerIndex, Player()));
        }
        catch (std::exception &e)
        {
            out << "Unable to host game!";
            return false;
        }
    }
    else
    {
        out << "A network game is already in progress, please disconnect first.";
        return false;
    }
    
    return true;
}

bool TanksGame::msgJoin(const unsigned int &, std::ostream &out, bool &, const unsigned int &ipHi, const unsigned int &ipLo, const unsigned int &port)
{
    if (!host && !client)
    {
        try
        {
            std::ostringstream ipAddress;
            
            //Extract IP address from two integers.
            ipAddress << ((ipHi/1000) % 1000) << "." << (ipHi % 1000) << "."
                      << ((ipLo/1000) % 1000) << "." << (ipLo % 1000);
            
            clear();
            client = new TanksClient(ipAddress.str(), port, this);
            out << "Joined game at " << ipAddress.str() << ":" << port << ".";
        }
        catch (std::exception &e)
        {
            out << "Unable to join game!";
            return false;
        }
    }
    else
    {
        out << "A network game is already in progress, please disconnect first.";
        return false;
    }
    
    return true;
}

bool TanksGame::msgDisconnect(const unsigned int &playerIndex, std::ostream &out, bool &)
{
    if (playerIndex == 0)
    {
        out << "Disconnecting...";
        clear();
    }
    else
    {
        out << "Player " << playerIndex << " tried to force us to disconnect!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgAddPlayer(const unsigned int &playerIndex, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
{
    if (playerIndex == 0)
    {
        if (players.find(newPlayerIndex) != players.end())
        {
            out << "Player with index " << newPlayerIndex << " will not be added, this player already exists!";
            return false;
        }
        else
        {
            if (host)
            {
                //Send this player to all other clients.
                if (true)
                {
                    Message msg(msg::mt::addPlayer);
                    
                    msg << newPlayerIndex;
                    host->sendMessage(msg);
                }
                
                //Send new client a welcome message.
                if (true)
                {
                    Message msg(msg::mt::welcomePlayer);
                    
                    msg << newPlayerIndex;
                    host->sendPrivateMessage(msg, newPlayerIndex);
                }
                
                //Send all current players to the new client.
                for (std::map<unsigned int, Player>::const_iterator j = players.begin(); j != players.end(); ++j)
                {
                    Message msg(msg::mt::addPlayer);
                    
                    msg << j->first;
                    host->sendPrivateMessage(msg, newPlayerIndex);
                }
                
                //TODO: Send game state to the new client.
            }
        
            players.insert(std::make_pair(newPlayerIndex, Player()));
            out << "Added player with index " << newPlayerIndex << ".";
        }
    }
    else
    {
        out << "Only the host can add players!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgRemovePlayer(const unsigned int &playerIndex, std::ostream &out, bool &, const unsigned int &oldPlayerIndex)
{
    if (playerIndex == 0)
    {
        std::map<unsigned int, Player>::iterator i = players.find(oldPlayerIndex);
        
        if (i == players.end())
        {
            out << "Player with index " << oldPlayerIndex << " will not be removed, this player does not exist!";
            return false;
        }
        else
        {
            if (host)
            {
                //Broadcast removal of this player.
                Message msg(msg::mt::removePlayer);
                
                msg << oldPlayerIndex;
                host->sendMessage(msg);
            }
            
            //Remove the player's tank.
            if (i->second.tankIndex > 0)
            {
                Message msg(msg::mt::removeTank);
                
                msg << i->second.tankIndex;
                applyMessage(0, msg);
            }
            
            players.erase(i);
            
            out << "Removed player with index " << oldPlayerIndex << ".";
        }
    }
    else
    {
        out << "Only the host can remove players!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgWelcomePlayer(const unsigned int &playerIndex, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
{
    if (playerIndex == 0 && client)
    {
        ownPlayerIndex = newPlayerIndex;
        out << "Welcomed to network game as player " << ownPlayerIndex << ".";
    }
    else
    {
        out << "Only clients receive welcome messages!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgTerrainOffset(const unsigned int &playerIndex, std::ostream &out, bool &broadcast, const vec2 &offset)
{
    if (playerIndex == 0)
    {
        broadcast = true;
        terrain->setOffset(offset);
        out << "Set terrain offset to " << offset << ".";
    }
    else
    {
        out << "Only the host can change the terrain offset!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgAddTank(const unsigned int &playerIndex, std::ostream &out, bool &broadcast, const unsigned int &tankIndex, const unsigned int &tankType, const vec2 &position)
{
    if (playerIndex == 0)
    {
        if (tankTypes.find(tankType) == tankTypes.end() || tanks.find(tankIndex) != tanks.end())
        {
            out << "Unknown tank type " << tankType << " or tank index " << tankIndex << " already in use!";
            return false;
        }
        else
        {
            TankInstance tank(tankType);
            
            tank.x = vec3(position.x, terrain->getHeight(position), position.y);
            tanks[tankIndex] = tank;
            if (lastTankIndex < tankIndex) lastTankIndex = tankIndex;
            
            out << "Added tank with index " << tankIndex << " of type " << tankType << ".";
            broadcast = true;
        }
    }
    else
    {
        out << "Only the host can create new tanks!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgRemoveTank(const unsigned int &playerIndex, std::ostream &out, bool &broadcast, const unsigned int &tankIndex)
{
    if (playerIndex == 0 || client)
    {
        if (tanks.find(tankIndex) == tanks.end())
        {
            out << "Tank with index " << tankIndex << " does not exist!";
            return false;
        }
        else
        {
            //TODO: Update tankIndex in Player.
            tanks.erase(tanks.find(tankIndex));
            out << "Removed tank with index " << tankIndex << ".";
            broadcast = true;
        }
    }
    else
    {
        out << "Only the host can remove tanks!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgUpdateTank(const unsigned int &playerIndex, std::ostream &out, bool &broadcast, const unsigned int &tankIndex,
                              const unsigned int &controls, const vec3 &x, const vec4 &q, const vec3 &P, const vec3 &L)
{
    //Clients are only permitted to modify the status of their own tanks.
    if (playerIndex == 0 || players[playerIndex].tankIndex == tankIndex)
    {
        std::map<unsigned int, TankInstance>::iterator i = tanks.find(tankIndex);
        
        if (i == tanks.end())
        {
            out << "Tank with index " << tankIndex << " does not exist!";
            return false;
        }
        else
        {
            //Update tank status.
            i->second.controls = controls;
            i->second.x = x;
            i->second.q = q;
            i->second.P = P;
            i->second.L = L;
            broadcast = true;
        }
    }
    else
    {
        out << "Clients cannot update tanks other than their own!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgSetPlayerTank(const unsigned int &playerIndex, std::ostream &out, bool &broadcast, const unsigned int &tankPlayerIndex, const unsigned int &tankIndex)
{
    if (playerIndex == 0)
    {
        if (tanks.find(tankIndex) == tanks.end() || players.find(tankPlayerIndex) == players.end())
        {
            out << "Player with index " << playerIndex << " or tank with index " << tankIndex << " does not exist!";
            return false;
        }
        else
        {
            players[tankPlayerIndex].tankIndex = tankIndex;
            out << "Assigned player " << tankPlayerIndex << " to tank " << tankIndex << ".";
            broadcast = true;
        }
    }
    else
    {
        out << "Only the host can reassign tank controllers!";
        return false;
    }
    
    return true;
}

bool TanksGame::msgPlayerSpawnRequest(const unsigned int &playerIndex, std::ostream &out, bool &, const unsigned int &tankType)
{
    if ((playerIndex == 0 || host) && !client)
    {
        if (tankTypes.find(tankType) == tankTypes.end())
        {
            out << "Tank type " << tankType << " does not exist!";
            return false;
        }
        else
        {
            //Create a new tank.
            const unsigned int tankIndex = lastTankIndex++;
            Message msg1(msg::mt::addTank);
            
            msg1 << tankIndex << tankType << vec2(0.0f, 0.0f);
            applyMessage(0, msg1);
            
            //Assign this player to that tank.
            Message msg2(msg::mt::setPlayerTank);
            
            msg2 << playerIndex << tankIndex;
            applyMessage(0, msg2);
            
            out << "Spawned player " << playerIndex << " in tank " << tankIndex << " of type " << tankType << ".";
        }
    }
    else
    {
        out << "Only the host can spawn players!";
        return false;
    }
    
    return true;
}

bool TanksGame::applyMessage(const unsigned int &playerIndex, const Message &message)
{
    std::ostringstream out;
    bool ok = true;
    //Do we want to send this message to all clients if we are the host?
    bool broadcast = false;
    
    if (host) assert(players.find(playerIndex) != players.end());
    
    if (message.id == msg::mt::none)
    {
        out << "No-operation message should never be sent.";
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
    else if (message.id == msg::mt::host) ok = msgHost(playerIndex, out, broadcast, message.data[0].iv1);
    else if (message.id == msg::mt::join) ok = msgJoin(playerIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].iv1);
    else if (message.id == msg::mt::disconnect) ok = msgDisconnect(playerIndex, out, broadcast);
    else if (message.id == msg::mt::addPlayer) ok = msgAddPlayer(playerIndex, out, broadcast, message.data[0].iv1);
    else if (message.id == msg::mt::removePlayer) ok = msgRemovePlayer(playerIndex, out, broadcast, message.data[0].iv1);
    else if (message.id == msg::mt::welcomePlayer) ok = msgWelcomePlayer(playerIndex, out, broadcast, message.data[0].iv1);
    else if (message.id == msg::mt::terrainOffset) ok = msgTerrainOffset(playerIndex, out, broadcast, message.data[0].v2);
    else if (message.id == msg::mt::addTank) ok = msgAddTank(playerIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v2);
    else if (message.id == msg::mt::removeTank) ok = msgRemoveTank(playerIndex, out, broadcast, message.data[0].iv1);
    else if (message.id == msg::mt::updateTank) ok = msgUpdateTank(playerIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v3, message.data[3].v4, message.data[4].v3, message.data[5].v3);
    else if (message.id == msg::mt::setPlayerTank) ok = msgSetPlayerTank(playerIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1);
    else if (message.id == msg::mt::playerSpawnRequest)
    {
        //Forward message to host if we are a client.
        if (client) client->sendMessage(message);
        else ok = msgPlayerSpawnRequest(playerIndex, out, broadcast, message.data[0].iv1);
    }
    else
    {
        out << "Message has unknown identifier " << message.id << "!";
        ok = false;
    }
    
    if (!out.str().empty()) console->addLine(out.str());
    
    if (broadcast && host)
    {
        //Broadcast this message to all clients.
        assert(ok);
        host->sendMessage(message);
    }
    
    return ok;
}

