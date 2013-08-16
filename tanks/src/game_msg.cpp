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

bool TanksGame::msgHelp(const unsigned int &, std::ostream &out, bool &)
{
    out << "\\w\\4==== Available commands:" << std::endl << translator->getMessageTypeNames("\n") << std::endl;
    out << "\\w\\4==== Full descriptions:" << std::endl << translator->getMessageTypeDescriptions();
    return true;
}

bool TanksGame::msgHost(const unsigned int &, std::ostream &out, bool &, const unsigned int &port)
{
    if (!host && !client)
    {
        try
        {
            clear();
            host = new TanksHost(port, this);
            out << "Hosting game at port " << port << ".";
            
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
            players.clear();
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

bool TanksGame::msgDisconnect(const unsigned int &, std::ostream &out, bool &)
{
    out << "Disconnecting...";
    clear();
    
    return true;
}

bool TanksGame::msgAddPlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
{
    if (players.find(newPlayerIndex) != players.end())
    {
        out << "Player with index " << newPlayerIndex << " will not be added, this player already exists!";
        return false;
    }
    
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
    
    return true;
}

bool TanksGame::msgRemovePlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &oldPlayerIndex)
{
    std::map<unsigned int, Player>::iterator i = players.find(oldPlayerIndex);
    
    if (i == players.end())
    {
        out << "Player with index " << oldPlayerIndex << " will not be removed, this player does not exist!";
        return false;
    }
    
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
    
    return true;
}

bool TanksGame::msgWelcomePlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
{
    if (client)
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

bool TanksGame::msgTerrainOffset(const unsigned int &, std::ostream &out, bool &broadcast, const vec2 &offset)
{
    terrain->setOffset(offset);
    out << "Set terrain offset to " << offset << ".";
    broadcast = true;
    
    return true;
}

bool TanksGame::msgAddTank(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &tankIndex, const unsigned int &tankType, const vec2 &position)
{
    if (tankTypes.find(tankType) == tankTypes.end() || tanks.find(tankIndex) != tanks.end())
    {
        out << "Unknown tank type " << tankType << " or tank index " << tankIndex << " already in use!";
        return false;
    }
    
    TankInstance tank(tankType);
    
    tank.x = vec3(position.x, terrain->getHeight(position), position.y);
    tanks[tankIndex] = tank;
    if (lastTankIndex < tankIndex) lastTankIndex = tankIndex;
    
    out << "Added tank with index " << tankIndex << " of type " << tankType << ".";
    broadcast = true;
    
    return true;
}

bool TanksGame::msgRemoveTank(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &tankIndex)
{
    if (tanks.find(tankIndex) == tanks.end())
    {
        out << "Tank with index " << tankIndex << " does not exist!";
        return false;
    }
    
    //TODO: Update tankIndex in Player.
    tanks.erase(tanks.find(tankIndex));
    out << "Removed tank with index " << tankIndex << ".";
    broadcast = true;
    
    return true;
}

bool TanksGame::msgUpdateTank(const unsigned int &senderIndex, std::ostream &out, bool &broadcast, const unsigned int &tankIndex,
                              const unsigned int &controls, const vec3 &x, const vec4 &q, const vec3 &P, const vec3 &L)
{
    //Clients are only permitted to modify the status of their own tanks.
    if (senderIndex != 0 && players[senderIndex].tankIndex != tankIndex)
    {
        out << "Player tried to send update for wrong tank!";
        return false;
    }
    
    std::map<unsigned int, TankInstance>::iterator i = tanks.find(tankIndex);
    
    if (i == tanks.end())
    {
        out << "Tank with index " << tankIndex << " does not exist!";
        return false;
    }
    
    //Update tank status.
    i->second.controls = controls;
    i->second.x = x;
    i->second.q = q;
    i->second.P = P;
    i->second.L = L;
    broadcast = true;
    
    return true;
}

bool TanksGame::msgSetPlayerTank(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &tankPlayerIndex, const unsigned int &tankIndex)
{
    if (tanks.find(tankIndex) == tanks.end() || players.find(tankPlayerIndex) == players.end())
    {
        out << "Player with index " << tankPlayerIndex << " or tank with index " << tankIndex << " does not exist!";
        return false;
    }
    
    players[tankPlayerIndex].tankIndex = tankIndex;
    out << "Assigned player " << tankPlayerIndex << " to tank " << tankIndex << ".";
    broadcast = true;
    
    return true;
}

bool TanksGame::msgPlayerSpawnRequest(const unsigned int &senderIndex, std::ostream &out, bool &, const unsigned int &tankType)
{
    if (tankTypes.find(tankType) == tankTypes.end())
    {
        out << "Tank type " << tankType << " does not exist!";
        return false;
    }
    
    //Create a new tank.
    const unsigned int tankIndex = lastTankIndex++;
    Message msg1(msg::mt::addTank);
    
    msg1 << tankIndex << tankType << vec2(0.0f, 0.0f);
    applyMessage(0, msg1);
    
    //Assign this player to that tank.
    Message msg2(msg::mt::setPlayerTank);
    
    msg2 << senderIndex << tankIndex;
    applyMessage(0, msg2);
    
    out << "Spawned player " << senderIndex << " in tank " << tankIndex << " of type " << tankType << ".";
    
    return true;
}

bool TanksGame::applyMessage(const unsigned int &senderIndex, const Message &message)
{
    std::ostringstream out;
    bool ok = true;
    //Do we want to send this message to all clients if we are the host?
    bool broadcast = false;
    
#ifndef NDEBUG
    if (host)
    {
        //The host should always have the indices of all players.
        assert(players.find(senderIndex) != players.end());
    }
#endif
    
    //Commands that only the host can give.
    if (senderIndex == 0)
    {
             if (message.id == msg::mt::help) ok = msgHelp(senderIndex, out, broadcast);
        else if (message.id == msg::mt::host) ok = msgHost(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::join) ok = msgJoin(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].iv1);
        else if (message.id == msg::mt::disconnect) ok = msgDisconnect(senderIndex, out, broadcast);
        else if (message.id == msg::mt::addPlayer) ok = msgAddPlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::removePlayer) ok = msgRemovePlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::welcomePlayer) ok = msgWelcomePlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::terrainOffset) ok = msgTerrainOffset(senderIndex, out, broadcast, message.data[0].v2);
        else if (message.id == msg::mt::addTank) ok = msgAddTank(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v2);
        else if (message.id == msg::mt::removeTank) ok = msgRemoveTank(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::updateTank) ok = msgUpdateTank(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v3, message.data[3].v4, message.data[4].v3, message.data[5].v3);
        else if (message.id == msg::mt::setPlayerTank) ok = msgSetPlayerTank(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1);
        else if (message.id == msg::mt::playerSpawnRequest) ok = msgPlayerSpawnRequest(senderIndex, out, broadcast, message.data[0].iv1);
    }
    else
    {
        //Host messages received from clients.
        assert(host && !client);
        
             if (message.id == msg::mt::updateTank) ok = msgUpdateTank(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v3, message.data[3].v4, message.data[4].v3, message.data[5].v3);
        else if (message.id == msg::mt::playerSpawnRequest) ok = msgPlayerSpawnRequest(senderIndex, out, broadcast, message.data[0].iv1);
    }
    
    //Add message output to the console.
    if (!out.str().empty())
    {
        console->addLine(out.str());
    }
    
    if (broadcast && host)
    {
        //Broadcast this message to all clients.
        assert(ok);
        host->sendMessage(message);
    }
    
    return ok;
}

bool TanksGame::userMessage(const Message &message)
{
    //Receive a command from the user.
    if (client)
    {
        //If we are a client, it is sent to the host.
        client->sendMessage(message);
    }
    else
    {
        //Otherwise, we apply it directly to the game and (possibly) let the host distribute it to the connected clients.
        return applyMessage(0, message);
    }
    
    return true;
}

