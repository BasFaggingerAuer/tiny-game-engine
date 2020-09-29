/*
Copyright 2020, Bas Fagginger Auer.

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
#include <iomanip>
#include <fstream>
#include <vector>
#include <exception>

#include "messages.h"
#include "game.h"

using namespace rpg;
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

bool Game::msgJoin(const unsigned int &, std::ostream &out, bool &, const std::string &ip, const unsigned int &port)
{
    if (!host && !client)
    {
        try
        {
            clear();
            players.clear();
            client = new GameClient(ip, port, this);
            out << "Joined game at " << ip << ":" << port << ".";
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
    
    //Remove the player's character.
    if (i->second.characterIndex > 0)
    {
        Message msg(msg::mt::removeCharacter);
        
        msg << i->second.characterIndex;
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

bool Game::msgSunDirection(const unsigned int &, std::ostream &out, bool &broadcast, const vec3 &direction)
{
    skyEffect->setSun(normalize(direction));
    
    out << "Set sun direction to " << direction << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgListCharacterTypes(const unsigned int &, std::ostream &out, bool &)
{
    out << "\\w\\4==== Available character types:" << std::endl;
    
    for (auto i = characterTypes.cbegin(); i != characterTypes.cend(); ++i)
    {
        out << std::setw(4) << std::setfill('0') << i->first << ": " << i->second->name << std::endl;
    }
    
    return true;
}

bool Game::msgListCharacters(const unsigned int &, std::ostream &out, bool &)
{
    out << "\\w\\4==== Available characters:" << std::endl;
    
    for (auto i = characters.cbegin(); i != characters.cend(); ++i)
    {
        out << std::setw(4) << std::setfill('0') << i->first << ": " << i->second.name << " (" << characterTypes[i->second.type]->name << ", " << std::setw(0) << i->second.type << ")" << std::endl;
    }
    
    return true;
}

bool Game::msgAddCharacter(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &characterIndex, const std::string &characterName, const unsigned int &characterTypeIndex, const float &color)
{
    if (characterTypes.find(characterTypeIndex) == characterTypes.end() || characters.find(characterIndex) != characters.end())
    {
        out << "Unknown character type " << characterTypeIndex << " or character index " << characterIndex << " already in use!";
        return false;
    }
    
    CharacterInstance character(characterTypeIndex, characterName, ivec3(0), 0, color);
    const CharacterType *characterType = characterTypes[characterTypeIndex];
    
    characters[characterIndex] = character;
    
    if (lastCharacterIndex < characterIndex) lastCharacterIndex = characterIndex;
    
    out << "Added character with index " << characterIndex << " of type '" << characterType->name << "' (" << characterTypeIndex << ").";
    broadcast = true;
    
    return true;
}

bool Game::msgRemoveCharacter(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &characterIndex)
{
    if (characters.find(characterIndex) == characters.end())
    {
        out << "Character with index " << characterIndex << " does not exist!";
        return false;
    }
    
    //TODO: Update characterIndex in Player.
    characters.erase(characters.find(characterIndex));
    out << "Removed character with index " << characterIndex << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgUpdateCharacter(const unsigned int &senderIndex, std::ostream &out, bool &broadcast, const unsigned int &characterIndex,
                              const ivec3 &position, const int &rotation, const float &color)
{
    //Clients are only permitted to modify the status of their own characters.
    if (senderIndex != 0 && players[senderIndex].characterIndex != characterIndex)
    {
        out << "Player tried to send update for wrong character!";
        return false;
    }
    
    std::map<unsigned int, CharacterInstance>::iterator i = characters.find(characterIndex);
    
    if (i == characters.end())
    {
        out << "Character with index " << characterIndex << " does not exist!";
        return false;
    }
    
    //Update character status, but only if it is a character different from the one you control as a client.
    if (senderIndex == 0 && players[ownPlayerIndex].characterIndex == characterIndex)
    {
        return true;
    }
    
    i->second.position = position;
    i->second.rotation = rotation;
    i->second.color = color;
    broadcast = true;
    
    return true;
}

bool Game::msgSetPlayerCharacter(const unsigned int &, std::ostream &out, bool &broadcast, const unsigned int &characterPlayerIndex, const unsigned int &characterIndex)
{
    if (players.find(characterPlayerIndex) == players.end())
    {
        out << "Player with index " << characterPlayerIndex << " does not exist!";
        return false;
    }
    
    if (characters.find(characterIndex) == characters.end() && characterIndex != 0)
    {
        out << "Player with index " << characterPlayerIndex << " or character with index " << characterIndex << " does not exist!";
        return false;
    }
    
    players[characterPlayerIndex].characterIndex = characterIndex;
    out << "Assigned player " << characterPlayerIndex << " to character " << characterIndex << ".";
    broadcast = true;
    
    return true;
}

bool Game::msgPlayerSpawnRequest(const unsigned int &senderIndex, std::ostream &out, bool &, const unsigned int &characterType)
{
    if (characterTypes.find(characterType) == characterTypes.end())
    {
        out << "Character type " << characterType << " does not exist!";
        return false;
    }
    
    //Create a new character.
    const unsigned int characterIndex = lastCharacterIndex++;
    Message msg1(msg::mt::addCharacter);
    
    msg1 << characterIndex << characterType << vec2(0.0f, 0.0f);
    applyMessage(0, msg1);
    
    //Assign this player to that character.
    Message msg2(msg::mt::setPlayerCharacter);
    
    msg2 << senderIndex << characterIndex;
    applyMessage(0, msg2);
    
    out << "Spawned player " << senderIndex << " in character " << characterIndex << " of type '" << characterTypes[characterType]->name << "' (" << characterType << ").";
    
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
        else if (message.id == msg::mt::join) ok = msgJoin(senderIndex, out, broadcast, message.data[0].s256, message.data[1].iv1);
        else if (message.id == msg::mt::disconnect) ok = msgDisconnect(senderIndex, out, broadcast);
        else if (message.id == msg::mt::addPlayer) ok = msgAddPlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::removePlayer) ok = msgRemovePlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::welcomePlayer) ok = msgWelcomePlayer(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::terrainOffset) ok = msgTerrainOffset(senderIndex, out, broadcast, message.data[0].v2);
        else if (message.id == msg::mt::sunDirection) ok = msgSunDirection(senderIndex, out, broadcast, message.data[0].v3);
        else if (message.id == msg::mt::listCharacterTypes) ok = msgListCharacterTypes(senderIndex, out, broadcast);
        else if (message.id == msg::mt::listCharacters) ok = msgListCharacters(senderIndex, out, broadcast);
        else if (message.id == msg::mt::addCharacter) ok = msgAddCharacter(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].s256, message.data[2].iv1, message.data[3].v1);
        else if (message.id == msg::mt::removeCharacter) ok = msgRemoveCharacter(senderIndex, out, broadcast, message.data[0].iv1);
        else if (message.id == msg::mt::updateCharacter) ok = msgUpdateCharacter(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv3, message.data[2].iv1, message.data[3].v1);
        else if (message.id == msg::mt::setPlayerCharacter) ok = msgSetPlayerCharacter(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv1);
        else if (message.id == msg::mt::playerSpawnRequest) ok = msgPlayerSpawnRequest(senderIndex, out, broadcast, message.data[0].iv1);
    }
    else
    {
        //Host messages received from clients.
        assert(host && !client);
        
        if (message.id == msg::mt::updateCharacter) ok = msgUpdateCharacter(senderIndex, out, broadcast, message.data[0].iv1, message.data[1].iv3, message.data[2].iv1, message.data[3].v1);
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

