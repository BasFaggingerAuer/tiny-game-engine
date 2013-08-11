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

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>

#include "messages.h"
#include "game.h"

using namespace tanks;
using namespace tiny;

Player::Player() :
    tankId(0)
{

}

Player::~Player()
{

}

TankType::TankType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading tank type resource..." << std::endl;
   
    assert(el->ValueStr() == "tank");
    
    name = "";
    nrInstances = 0;
    maxNrInstances = 0;
     
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("name", &name);
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read mesh and textures.
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + normalFileName));
    horde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(path + meshFileName), maxNrInstances);
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' tank type." << std::endl;
}

TankType::~TankType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
}

void TankType::clearInstances()
{
    nrInstances = 0;
}

void TankType::addInstance(const TankInstance &tank)
{
    if (nrInstances < maxNrInstances)
    {
        instances[nrInstances++] = draw::StaticMeshInstance(vec4(tank.pos.x, tank.pos.y, tank.pos.z, 1.0f), tank.ori);
    }
}

void TankType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
}

TanksGame::TanksGame(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight())),
    lastTankIndex(1),
    translator(new TanksMessageTranslator()),
    console(new TanksConsole(this)),
    host(0),
    client(0),
    ownPlayerIndex(0)
{
    readResources(path);
    cameraPosition = vec3(0.0f, 0.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    consoleMode = false;
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    renderer->addWorldRenderable(skyBoxMesh);
    renderer->addWorldRenderable(terrain->terrain);
    
    for (std::map<unsigned int, TankType *>::const_iterator i = tankTypes.begin(); i != tankTypes.end(); ++i)
    {
        renderer->addWorldRenderable(i->second->horde);
    }
    
    renderer->addScreenRenderable(skyEffect, false, false);
    renderer->addScreenRenderable(font, false, false, draw::BlendMix);
}

TanksGame::~TanksGame()
{
    disconnect();
    
    delete translator;
    delete renderer;
    
    delete font;
    delete fontTexture;
    
    delete skyBoxMesh;
    delete skyBoxDiffuseTexture;
    delete skyGradientTexture;
    delete skyEffect;
    
    delete terrain;
    
    for (std::map<unsigned int, TankType *>::iterator i = tankTypes.begin(); i != tankTypes.end(); ++i)
    {
        delete i->second;
    }
}

void TanksGame::updateConsole() const
{
    //Update the console on screen.
    font->setText(-1.0, -1.0, consoleMode ? 0.05 : 0.025, aspectRatio, console->getText(256), *fontTexture);
}

void TanksGame::update(os::Application *application, const double &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    if (application->isKeyPressedOnce('`'))
    {
        consoleMode = !consoleMode;
        updateConsole();
    }
    
    if (consoleMode)
    {
        //Update console.
        for (int i = 0; i < 256; ++i)
        {
            if (application->isKeyPressedOnce(i))
            {
                console->keyDown(i);
            }
        }
    }
    else
    {
        //Do we control a tank?
        unsigned int tankId = 0;
        
        if (players.find(ownPlayerIndex) != players.end())
        {
            tankId = players[ownPlayerIndex].tankId;
            
            if (tanks.find(tankId) == tanks.end())
            {
                //If we have received a nonzero tank id, it should always be valid!
                assert(tankId == 0);
                tankId = 0;
            }
        }
        
        if (tankId == 0)
        {
            //We do not control a tank: control the camera directly.
            //Move the camera around and collide with the terrain.
            application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
            cameraPosition.y = std::max(cameraPosition.y, terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f);
        }
        else
        {
            //Update the controls of our tank and send these to the host if we are a client.
            unsigned int controls = 0;
            
            if (application->isKeyPressed('w')) controls |=   1;
            if (application->isKeyPressed('s')) controls |=   2;
            if (application->isKeyPressed('a')) controls |=   4;
            if (application->isKeyPressed('d')) controls |=   8;
            if (application->isKeyPressed('i')) controls |=  16;
            if (application->isKeyPressed('j')) controls |=  32;
            if (application->isKeyPressed('k')) controls |=  64;
            if (application->isKeyPressed('l')) controls |= 128;
            
            TankInstance &tank = tanks[tankId];
            
            if (controls != tank.controls)
            {
                tank.controls = controls;
                
                //Is this a network game?
                if (host || client)
                {
                    Message msg(msg::mt::updateTank);
                    
                    msg << static_cast<int>(tankId) << static_cast<int>(tank.controls) << tank.pos << tank.ori << tank.vel << tank.omega;
                    
                    if (client) client->sendMessage(msg);
                    if (host) host->sendMessage(msg);
                }
            }
            
            //Look from our tank.
            cameraPosition = tank.pos;
            cameraOrientation = tank.ori;
        }
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
    }
    
    //Update all tanks.
    for (std::map<unsigned int, TankInstance>::iterator i = tanks.begin(); i != tanks.end(); ++i)
    {
        TankInstance &t = i->second;
        
        if (t.controls & 1) t.pos.x += 4.0f*dt;
        if (t.controls & 2) t.pos.x -= 4.0f*dt;
    }

    //Draw all tanks.
    for (std::map<unsigned int, TankType *>::iterator i = tankTypes.begin(); i != tankTypes.end(); ++i)
    {
        i->second->clearInstances();
    }
    
    for (std::map<unsigned int, TankInstance>::const_iterator i = tanks.begin(); i != tanks.end(); ++i)
    {
        assert(tankTypes.find(i->second.type) != tankTypes.end());
        tankTypes[i->second.type]->addInstance(i->second);
    }
    
    for (std::map<unsigned int, TankType *>::iterator i = tankTypes.begin(); i != tankTypes.end(); ++i)
    {
        i->second->updateInstances();
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
    //Do we want to send this message to all clients if we are the host?
    bool broadcast = false;
    
    if (host) assert(players.find(playerIndex) != players.end());
    
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
                
                //Enter the game as own player.
                ownPlayerIndex = 0;
                players.insert(std::make_pair(ownPlayerIndex, Player()));
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
            ownPlayerIndex = message.data[0].iv1;
            out << "Welcomed to network game as player " << ownPlayerIndex << ".";
        }
        else
        {
            out << "Only clients receive welcome messages!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::terrainOffset)
    {
        if (playerIndex == 0 || client)
        {
            broadcast = true;
            terrain->setOffset(message.data[0].v2);
            out << "Set terrain offset to " << message.data[0].v2 << ".";
        }
        else
        {
            out << "Only the host can change the terrain offset!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::addTank)
    {
        if (playerIndex == 0 || client)
        {
            const unsigned int tankId = message.data[0].iv1;
            const unsigned int tankType = message.data[1].iv1;
            const vec2 tankPos = message.data[2].v2;
            
            if (tankTypes.find(tankType) == tankTypes.end() || tanks.find(tankId) != tanks.end())
            {
                out << "Unknown tank type " << tankType << " or tank id " << tankId << " already in use!";
                ok = false;
            }
            else
            {
                TankInstance tank(tankType);
                
                tank.pos = vec3(tankPos.x, terrain->getHeight(tankPos), tankPos.y);
                tanks[tankId] = tank;
                out << "Added tank with id " << tankId << " of type " << tankType << ".";
                broadcast = true;
            }
        }
        else
        {
            out << "Only the host can create new tanks!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::removeTank)
    {
        if (playerIndex == 0 || client)
        {
            const unsigned int tankId = message.data[0].iv1;
            
            if (tanks.find(tankId) == tanks.end())
            {
                out << "Tank with id " << tankId << " does not exist!";
                ok = false;
            }
            else
            {
                //TODO: Update tankId in Player.
                tanks.erase(tanks.find(tankId));
                out << "Removed tank with id " << tankId << ".";
                broadcast = true;
            }
        }
        else
        {
            out << "Only the host can remove tanks!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::updateTank)
    {
        //Clients are only permitted to modify the status of their own tanks.
        if ((client && playerIndex != ownPlayerIndex) || (host && players[playerIndex].tankId == static_cast<unsigned int>(message.data[0].iv1)))
        {
            std::map<unsigned int, TankInstance>::iterator i = tanks.find(message.data[0].iv1);
            
            if (i == tanks.end())
            {
                out << "Tank with id " << message.data[0].iv1 << " does not exist!";
                ok = false;
            }
            else
            {
                //Update tank status.
                i->second.controls = message.data[1].iv1;
                i->second.pos = message.data[2].v3;
                i->second.ori = message.data[3].v4;
                i->second.vel = message.data[4].v3;
                i->second.omega = message.data[5].v3;
                broadcast = true;
            }
        }
        else
        {
            out << "Clients cannot update tanks other than their own!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::setPlayerTank)
    {
        if (playerIndex == 0 || client)
        {
            const unsigned int playerId = message.data[0].iv1;
            const unsigned int tankId = message.data[1].iv1;
            
            if ((tanks.find(tankId) == tanks.end() && tankId != 0) || players.find(playerId) == players.end())
            {
                out << "Player with id " << playerId << " or tank with id " << tankId << " does not exist!";
                ok = false;
            }
            else
            {
                players[playerId].tankId = tankId;
                out << "Assigned player " << playerId << " to tank " << tankId << ".";
                broadcast = true;
            }
        }
        else
        {
            out << "Only the host can reassign tank controllers!";
            ok = false;
        }
    }
    else if (message.id == msg::mt::playerSpawnRequest)
    {
        if (host)
        {
            const unsigned int tankTypeId = message.data[0].iv1;
            
            if (tankTypes.find(tankTypeId) == tankTypes.end())
            {
                out << "Tank type with id " << tankTypeId << " does not exist!";
                ok = false;
            }
            else
            {
                //Create a new tank.
                const unsigned int tankId = lastTankIndex++;
                Message msg1(msg::mt::addTank);
                
                msg1 << static_cast<int>(tankId) << static_cast<int>(tankTypeId) << vec2(0.0f, 0.0f);
                applyMessage(0, msg1);
                
                //Assign this player to that tank.
                Message msg2(msg::mt::setPlayerTank);
                
                msg2 << static_cast<int>(playerIndex) << static_cast<int>(tankId);
                applyMessage(0, msg2);
                
                out << "Spawned player " << playerIndex << " in tank " << tankId << " of type " << tankTypeId << ".";
            }
        }
        else
        {
            out << "Only the host can spawn players!";
            ok = false;
        }
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
    ownPlayerIndex = 0;
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

void TanksGame::readResources(const std::string &path)
{
    const std::string worldFileName = path + "tanks.xml";
    
    std::cerr << "Reading resources from '" << worldFileName << "'." << std::endl;
    
    //Read world description from an XML file.
    TiXmlDocument doc(worldFileName.c_str());
    
    if (!doc.LoadFile())
    {
        std::cerr << "Unable to read XML file '" << worldFileName << "'!" << std::endl;
        throw std::exception();
    }
    
    TiXmlElement *root = doc.FirstChildElement();
    
    if (!root)
    {
        std::cerr << "No XML root node!" << std::endl;
        throw std::exception();
    }
    
    if (root->ValueStr() != "tanks")
    {
        std::cerr << "This is not a world of tanks world XML file!" << std::endl;
        throw std::exception();
    }
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
             if (el->ValueStr() == "console") readConsoleResources(path, el);
        else if (el->ValueStr() == "sky") readSkyResources(path, el);
        else if (el->ValueStr() == "terrain") terrain = new TanksTerrain(path, el);
        else if (el->ValueStr() == "tank") tankTypes[tankTypes.size()] = new TankType(path, el);
    }
}

void TanksGame::readConsoleResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading console resources..." << std::endl;
    
    int fontTextureSize = 0;
    int fontSize = 0;
    std::string fontFileName = "";
    
    assert(el->ValueStr() == "console");
    
    el->QueryIntAttribute("texture_size", &fontTextureSize);
    el->QueryIntAttribute("font_size", &fontSize);
    el->QueryStringAttribute("font", &fontFileName);
    
    //Read font from disk as a texture.
    fontTexture = new draw::IconTexture2D(fontTextureSize, fontTextureSize);
    fontTexture->packIcons(img::io::readFont(path + fontFileName, fontSize));
    
    //Create a drawable font object as a collection of instanced screen icons.
    font = new draw::ScreenIconHorde(4096);
    font->setIconTexture(*fontTexture);
    font->setText(-1.0, -1.0, 0.1, aspectRatio, "Welcome to \\ra Game of Tanks\\w, press ` to access the console.", *fontTexture);
}

void TanksGame::readSkyResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading sky resources..." << std::endl;
    
    std::string textureFileName = "";
    
    assert(el->ValueStr() == "sky");
    
    el->QueryStringAttribute("texture", &textureFileName);
    
    //Create sky box mesh and read gradient texture.
    skyBoxMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
    skyBoxDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    skyBoxMesh->setDiffuseTexture(*skyBoxDiffuseTexture);
    
    skyEffect = new draw::effects::SunSky();
    skyGradientTexture = new draw::RGBTexture2D(img::io::readImage(path + textureFileName), draw::tf::filter);
    skyEffect->setSkyTexture(*skyGradientTexture);
}

