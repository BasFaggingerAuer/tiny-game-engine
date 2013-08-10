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
using namespace tiny;

Player::Player()
{

}

Player::~Player()
{

}

TanksGame::TanksGame(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight())),
    translator(new TanksMessageTranslator()),
    console(new TanksConsole(this)),
    host(0),
    client(0),
    clientPlayerIndex(0)
{
    readResources(path);
    cameraPosition = vec3(0.0f, 0.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    consoleMode = false;
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    renderer->addWorldRenderable(skyBoxMesh);
    renderer->addWorldRenderable(terrain);
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
    delete terrainHeightTexture;
    delete terrainFarHeightTexture;
    delete terrainNormalTexture;
    delete terrainFarNormalTexture;
    delete terrainTangentTexture;
    delete terrainFarTangentTexture;
    delete terrainAttributeTexture;
    delete terrainFarAttributeTexture;
    
    delete biomeDiffuseTextures;
    delete biomeNormalTextures;
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
        //Move the camera around.
        application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
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
            out << "Welcomed to network game as player " << clientPlayerIndex << ".";
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
        else if (el->ValueStr() == "terrain") readTerrainResources(path, el);
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

void TanksGame::setTerrainOffset(const vec2 &offset)
{
    //Shifts the local heightmap to a specific spot in the global heightmap.
    terrainFarOffset = offset;
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*terrainFarHeightTexture, *terrainHeightTexture,
                                vec2(1.0f/static_cast<float>(terrainFarScale.x), 1.0f/static_cast<float>(terrainFarScale.y)),
                                offset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*terrainHeightTexture, *terrainHeightTexture, terrainFarScale.x);
    
    //Calculate normal and tangent maps.
    draw::computeNormalMap(*terrainFarHeightTexture, *terrainFarNormalTexture, terrainScale.x*terrainFarScale.x);
    draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
    draw::computeTangentMap(*terrainFarHeightTexture, *terrainFarTangentTexture, terrainScale.x*terrainFarScale.x);
    draw::computeTangentMap(*terrainHeightTexture, *terrainTangentTexture, terrainScale.x);
    
    //We do this for both the zoomed-in and far-away terrain.
    
    //TODO.
    //computeTerrainTypeFromHeight(*terrainHeightTexture, *terrainAttributeTexture, terrainScale.x);
    //computeTerrainTypeFromHeight(*terrainFarHeightTexture, *terrainFarAttributeTexture, terrainScale.x*terrainFarScale.x);
    
    //Retrieve all generated data.
    terrainHeightTexture->getFromDevice();
    terrainAttributeTexture->getFromDevice();
    
    //Set proper textures in the terrain.
    terrain->setFarHeightTextures(*terrainHeightTexture, *terrainFarHeightTexture,
                                  *terrainTangentTexture, *terrainFarTangentTexture,
                                  *terrainNormalTexture, *terrainFarNormalTexture,
                                  terrainScale, terrainFarScale, terrainFarOffset);
    terrain->setFarDiffuseTextures(*terrainAttributeTexture, *terrainFarAttributeTexture,
                                   *biomeDiffuseTextures, *biomeNormalTextures,
                                   terrainDetailScale);
}

void TanksGame::readTerrainResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading terrain resources..." << std::endl;
    
    std::vector<img::Image> diffuseTextures;
    std::vector<img::Image> normalTextures;
    img::Image heightMap;
    std::string attributeMapShader = "";
    float widthScale = 1.0f;
    float heightScale = 1.0f;
    int farScale = 2; 
    float detailScale = 1.0f;
    
    assert(el->ValueStr() == "terrain");
    
    el->QueryFloatAttribute("scale_width", &widthScale);
    el->QueryFloatAttribute("scale_height", &heightScale);
    el->QueryIntAttribute("scale_far", &farScale);
    el->QueryFloatAttribute("scale_detail", &detailScale);
    if (el->Attribute("heightmap")) heightMap = img::io::readImage(path + std::string(el->Attribute("heightmap")));
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "biome")
        {
            if (sl->Attribute("diffuse")) diffuseTextures.push_back(img::io::readImage(path + std::string(sl->Attribute("diffuse"))));
            if (sl->Attribute("normal")) normalTextures.push_back(img::io::readImage(path + std::string(sl->Attribute("normal"))));
        }
        else if (sl->ValueStr() == "shader")
        {
            attributeMapShader = std::string(sl->GetText());
        }
    }
    
    //Actually construct the terrain using all the data that we have.
    terrainScale = vec2(widthScale);
    terrainFarScale = ivec2(farScale);
    terrainDetailScale = vec2(detailScale);
    terrainHeightTexture = new draw::FloatTexture2D(heightMap, draw::tf::filter);
    terrainFarHeightTexture = new draw::FloatTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight(), draw::tf::filter);
    
    //Scale vertical range of the far-away heightmap.
    draw::computeScaledTexture(*terrainHeightTexture, *terrainFarHeightTexture, vec4(heightScale/255.0f), vec4(0.0f));
    
    //Create normal maps for the far-away and zoomed-in heightmaps.
    terrainFarNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainFarTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    //Create an attribute texture that determines the terrain type (forest/grass/mud/stone) based on the altitude and slope.
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth(), 0, 0, 0, 0));
    terrainFarAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    
    //Read diffuse textures and make them tileable.
    biomeDiffuseTextures = new draw::RGBTexture2DArray(diffuseTextures.begin(), diffuseTextures.end());
    biomeNormalTextures = new draw::RGBTexture2DArray(normalTextures.begin(), normalTextures.end());
    
    //Paint the terrain with the zoomed-in and far-away textures.
    terrain = new draw::Terrain(6, 8);
    
    setTerrainOffset(vec2(0.5f));
}

