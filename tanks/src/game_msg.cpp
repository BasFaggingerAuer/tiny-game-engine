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

bool Game::msgHelp(const unsigned int &, std::ostream &out, bool &)
{
    out << "\\w\\4==== Available commands:" << std::endl << translator->getMessageTypeNames("\n") << std::endl;
    out << "\\w\\4==== Full descriptions:" << std::endl << translator->getMessageTypeDescriptions();
    return true;
}

bool Game::msgHost(const unsigned int &, std::ostream &out, bool &, const unsigned int &port)
{
    if (!host && !client)
    {
        try
        {
            clear();
            host = new GameHost(port, this);
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

bool Game::msgJoin(const unsigned int &, std::ostream &out, bool &, const unsigned int &ipHi, const unsigned int &ipLo, const unsigned int &port)
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
            client = new GameClient(ipAddress.str(), port, this);
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

bool Game::msgDisconnect(const unsigned int &, std::ostream &out, bool &)
{
    out << "Disconnecting...";
    clear();
    
    return true;
}

bool Game::msgAddPlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
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

bool Game::msgRemovePlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &oldPlayerIndex)
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
    
    //Remove the player's soldier.
    if (i->second.soldierIndex > 0)
    {
        Message msg(msg::mt::removeSoldier);
        
        msg << i->second.soldierIndex;
        applyMessage(0, msg);
    }
    
    players.erase(i);
    out << "Removed player with index " << oldPlayerIndex << ".";
    
    return true;
}

bool Game::msgWelcomePlayer(const unsigned int &, std::ostream &out, bool &, const unsigned int &newPlayerIndex)
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

bool Game::msgTerrainOffset(const unsigned int &, std::ostream &out, bool &broadcast, const vec2 &offset)
{
    terrain->setOffset(offset);
    out << "Set terrain offset to " << offset << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgAddSoldier(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &soldierIndex, const unsigned int &soldierTypeIndex, const vec2 &position)
{
    if (soldierTypes.find(soldierTypeIndex) == soldierTypes.end() || soldiers.find(soldierIndex) != soldiers.end())
    {
        out << "Unknown soldier type " << soldierTypeIndex << " or soldier index " << soldierIndex << " already in use!";
        return false;
    }
    
    SoldierInstance soldier(soldierTypeIndex);
    const SoldierType *soldierType = soldierTypes[soldierTypeIndex];
    
    soldier.x = vec3(position.x, terrain->getHeight(position), position.y);
    soldier.weaponRechargeTimes.assign(soldierType->weapons.size(), 0.0f);
    
    soldiers[soldierIndex] = soldier;
    if (lastSoldierIndex < soldierIndex) lastSoldierIndex = soldierIndex;
    
    out << "Added soldier with index " << soldierIndex << " of type '" << soldierType->name << "' (" << soldierTypeIndex << ").";
    broadcast = true;
    
    return true;
}

bool Game::msgRemoveSoldier(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &soldierIndex)
{
    if (soldiers.find(soldierIndex) == soldiers.end())
    {
        out << "Soldier with index " << soldierIndex << " does not exist!";
        return false;
    }
    
    //TODO: Update soldierIndex in Player.
    soldiers.erase(soldiers.find(soldierIndex));
    out << "Removed soldier with index " << soldierIndex << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgUpdateSoldier(const unsigned int &senderIndex, std::ostream &out, bool &broadcast, const unsigned int &soldierIndex,
                                 const unsigned int &controls, const vec2 &angles, const vec3 &x, const vec4 &q, const vec3 &P)
{
    //Clients are only permitted to modify the status of their own soldiers.
    if (senderIndex != 0 && players[senderIndex].soldierIndex != soldierIndex)
    {
        out << "Player tried to send update for wrong soldier!";
        return false;
    }
    
    std::map<unsigned int, SoldierInstance>::iterator i = soldiers.find(soldierIndex);
    
    if (i == soldiers.end())
    {
        out << "Soldier with index " << soldierIndex << " does not exist!";
        return false;
    }
    
    //Update soldier status, but only if it is a soldier different from the one you control as a client.
    if (senderIndex == 0 && players[ownPlayerIndex].soldierIndex == soldierIndex)
    {
        return true;
    }
    
    i->second.controls = controls;
    i->second.angles = angles;
    i->second.x = x;
    i->second.q = q;
    i->second.P = P;
    broadcast = true;
    
    return true;
}

bool Game::msgSetPlayerSoldier(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &soldierPlayerIndex, const unsigned int &soldierIndex)
{
    if (soldiers.find(soldierIndex) == soldiers.end() || players.find(soldierPlayerIndex) == players.end())
    {
        out << "Player with index " << soldierPlayerIndex << " or soldier with index " << soldierIndex << " does not exist!";
        return false;
    }
    
    players[soldierPlayerIndex].soldierIndex = soldierIndex;
    out << "Assigned player " << soldierPlayerIndex << " to soldier " << soldierIndex << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgPlayerSpawnRequest(const unsigned int &senderIndex, std::ostream &out, bool &, const unsigned int &soldierType)
{
    if (soldierTypes.find(soldierType) == soldierTypes.end())
    {
        out << "Soldier type " << soldierType << " does not exist!";
        return false;
    }
    
    //Create a new soldier.
    const unsigned int soldierIndex = lastSoldierIndex++;
    Message msg1(msg::mt::addSoldier);
    
    msg1 << soldierIndex << soldierType << vec2(0.0f, 0.0f);
    applyMessage(0, msg1);
    
    //Assign this player to that soldier.
    Message msg2(msg::mt::setPlayerSoldier);
    
    msg2 << senderIndex << soldierIndex;
    applyMessage(0, msg2);
    
    out << "Spawned player " << senderIndex << " in soldier " << soldierIndex << " of type '" << soldierTypes[soldierType]->name << "' (" << soldierType << ").";
    
    return true;
}

bool Game::msgPlayerShootRequest(const unsigned int &senderIndex, std::ostream &out, bool &, const unsigned int &weaponIndex)
{
    //Does the player control a soldier which can fire bullets of this type?
    const unsigned int soldierIndex = players[senderIndex].soldierIndex;
    
    if (soldiers.find(soldierIndex) == soldiers.end())
    {
        out << "Player " << senderIndex << " sent a shoot request while not controlling a soldier!";
        return false;
    }
    
    //Retrieve soldier.
    SoldierInstance &soldier = soldiers[soldierIndex];
    const SoldierType *soldierType = soldierTypes[soldier.type];
    
    if (weaponIndex >= soldierType->weapons.size())
    {
        out << "Player " << senderIndex << " sent a shoot request for an invalid weapon index " << weaponIndex << "!";
        return false;
    }
    
    const unsigned int bulletType = soldierType->weapons[weaponIndex].bulletType;
    
    //Does the bullet type exist?
    if (bulletTypes.find(bulletType) == bulletTypes.end())
    {
        out << "Bullet type " << bulletType << " does not exist!";
        return false;
    }
    
    //Is the weapon charged?
    if (soldier.weaponRechargeTimes[weaponIndex] > 0.0f)
    {
        out << "Player " << senderIndex << " sent a shoot request for an uncharged weapon " << soldier.weaponRechargeTimes[weaponIndex] << "!";
        return false;
    }
    
    //Fire bullet.
    soldier.weaponRechargeTimes[weaponIndex] = soldierType->weapons[weaponIndex].rechargeTime;
    
    //Create a new bullet.
    const unsigned int bulletIndex = lastBulletIndex++;
    Message msg1(msg::mt::addBullet);
    
    msg1 << bulletIndex << bulletType;
    
    //Obtain current viewing direction and position of the soldier.
    const vec3 pos = soldierType->getCameraPosition(soldier);
    const mat4 ori = mat4(soldierType->getCameraOrientation(soldier));
    
    msg1 << (pos + ori*bulletTypes[bulletType]->position);
    msg1 << ((soldier.P/soldierType->mass) + ori*bulletTypes[bulletType]->velocity);
    msg1 << (ori*bulletTypes[bulletType]->acceleration);
    
    applyMessage(0, msg1);
    
    out << "Shot bullet of type '" << bulletTypes[bulletType]->name << "' (" << bulletType << ") for player " << senderIndex << " from soldier " << soldierIndex << ".";
    
    return true;
}

bool Game::msgAddBullet(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &bulletIndex, const unsigned int &bulletType, const vec3 &position, const vec3 &velocity, const vec3 &acceleration)
{
    if (bulletTypes.find(bulletType) == bulletTypes.end() || bullets.find(bulletIndex) != bullets.end())
    {
        out << "Unknown bullet type " << bulletType << " or bullet index " << bulletIndex << " already in use!";
        return false;
    }
    
    BulletInstance bullet(bulletType);
    const BulletType *type = bulletTypes[bulletType];
    
    bullet.lifetime = type->lifetime;
    bullet.x = position;
    bullet.v = velocity;
    bullet.a = acceleration;
    
    bullets[bulletIndex] = bullet;
    
    if (lastBulletIndex < bulletIndex) lastBulletIndex = bulletIndex;
    
    out << "Added bullet with index " << bulletIndex << " of type '" << bulletTypes[bulletType]->name << "' (" << bulletType << ").";
    broadcast = true;
    
    //Create sound effect.
    const tiny::snd::Sample *sound = type->shootSound;
    
    if (sound)
    {
        tiny::snd::playSample(*sound, -1, 0);
    }
    
    return true;
}

bool Game::applyMessage(const unsigned int &senderIndex, const Message &message)
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
        else if (message.id == msg::mt::addSoldier) ok = msgAddSoldier(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v2);
        else if (message.id == msg::mt::removeSoldier) ok = msgRemoveSoldier(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::updateSoldier) ok = msgUpdateSoldier(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v2, message.data[3].v3, message.data[4].v4, message.data[5].v3);
        else if (message.id == msg::mt::setPlayerSoldier) ok = msgSetPlayerSoldier(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1);
        else if (message.id == msg::mt::playerSpawnRequest) ok = msgPlayerSpawnRequest(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::playerShootRequest) ok = msgPlayerShootRequest(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::addBullet) ok = msgAddBullet(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v3, message.data[3].v3, message.data[4].v3);
    }
    else
    {
        //Host messages received from clients.
        assert(host && !client);
        
             if (message.id == msg::mt::updateSoldier) ok = msgUpdateSoldier(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1, message.data[2].v2, message.data[3].v3, message.data[4].v4, message.data[5].v3);
        else if (message.id == msg::mt::playerSpawnRequest) ok = msgPlayerSpawnRequest(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::playerShootRequest) ok = msgPlayerShootRequest(senderIndex, out, broadcast, message.data[0].iv1);
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

bool Game::userMessage(const Message &message)
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

