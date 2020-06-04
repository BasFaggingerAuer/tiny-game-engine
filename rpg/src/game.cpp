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
#include <algorithm>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/snd/worldsounderer.h>

#include <cstdio>
#include <cstdlib>
#include <sstream>

#include "messages.h"
#include "game.h"

using namespace rpg;
using namespace tiny;

Player::Player() :
    characterIndex(0)
{

}

Player::~Player()
{

}

CharacterType::CharacterType(const std::string &, TiXmlElement *el)
{
    std::cerr << "Reading character type resource..." << std::endl;
   
    assert(std::string(el->Value()) == "charactertype");
    
    name = "";
    size = vec3(1.0f);
    
    nrInstances = 0;
    maxNrInstances = 1;
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("width", &size.x);
    el->QueryFloatAttribute("height", &size.y);
    el->QueryFloatAttribute("depth", &size.z);
    
    /*
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    */
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read mesh and textures.
    /*
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + normalFileName));
    horde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(path + meshFileName), maxNrInstances);
    */
    diffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    normalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    horde = new draw::StaticMeshHorde(mesh::StaticMesh::createBoxMesh(size.x, size.y, size.z), maxNrInstances);
    
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' character type." << std::endl;
}

CharacterType::~CharacterType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
}

void CharacterType::clearInstances()
{
    nrInstances = 0;
}

void CharacterType::addInstance(const CharacterInstance &instance, const float &baseHeight)
{
    if (nrInstances < maxNrInstances - 1)
    {
        instances[nrInstances++] = draw::StaticMeshInstance(vec4(instance.position.x, instance.position.y + baseHeight + size.y, instance.position.z, 1.0f),
                                                            quatrot(instance.rotation, vec3(0.0f, 1.0f, 0.0f)),
                                                            instance.getColor());
        instances[nrInstances++] = draw::StaticMeshInstance(vec4(instance.position.x, baseHeight + 0.1f - size.y, instance.position.z, 1.0f),
                                                            quatrot(instance.rotation, vec3(0.0f, 1.0f, 0.0f)),
                                                            instance.getColor());
    }
}

void CharacterType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
}

Game::Game(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight())),
    mouseSensitivity(48.0),
    gravitationalConstant(9.81),
    lastSoundSourceIndex(1),
    lastCharacterIndex(1),
    translator(new GameMessageTranslator()),
    console(new GameConsole(this)),
    host(0),
    client(0),
    ownPlayerIndex(0)
{
    readResources(path);
    consoleMode = false;
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    unsigned int index = 0;
    
    renderer->addWorldRenderable(index++, skyBoxMesh);
    renderer->addWorldRenderable(index++, terrain->terrain);
    
    for (std::map<unsigned int, CharacterType *>::const_iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        renderer->addWorldRenderable(index++, i->second->horde);
    }
    
    renderer->addScreenRenderable(index++, skyEffect, false, false);
    renderer->addScreenRenderable(index++, fontWorld, false, false, draw::BlendMix);
    renderer->addScreenRenderable(index++, consoleBackground, false, false, draw::BlendMix);
    renderer->addScreenRenderable(index++, font, false, false, draw::BlendMix);
    
    clear();
}

Game::~Game()
{
    clear();
    
    delete translator;
    delete renderer;
    
    delete consoleBackground;
    delete font;
    delete fontTexture;
    
    delete skyBoxMesh;
    delete skyBoxDiffuseTexture;
    delete skyGradientTexture;
    delete skyEffect;
    
    delete terrain;
    
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        delete i->second;
    }
}

void Game::updateConsole() const
{
    if (consoleMode)
    {
        //Update the console on screen.
        font->setText(-1.0, -1.0, 0.05, aspectRatio, console->getText(256), *fontTexture);
        consoleBackground->setColour(vec4(0.0f, 0.0f, 0.0f, 0.8f));
        consoleBackground->setSquareDimensions(-1.0f, 1.0f, 1.0f, -1.0f);
    }
    else
    {
        //Put characters in a string.
        std::stringstream strm;
        
        for (auto i = characters.begin(); i != characters.end(); ++i)
        {
            const vec4 c = i->second.getColor();
            
            strm << "\\#fff" << i->first << " (" << std::dec << std::setprecision(0) << std::fixed << 5.0f*i->second.position.x << ", " << 5.0f*i->second.position.y << ", " << 5.0f*i->second.position.z << "): \\#" << std::hex << std::min(15, static_cast<int>(16.0f*c.x)) << std::min(15, static_cast<int>(16.0f*c.y)) << std::min(15, static_cast<int>(16.0f*c.z)) << i->second.name << std::endl;
        }
        
        font->setText(-1.0, -1.0, 0.075, aspectRatio, strm.str(), *fontTexture);
        consoleBackground->setColour(vec4(0.0f, 0.0f, 0.0f, 0.4f));
        consoleBackground->setSquareDimensions(-1.0f, 0.0f, -0.3f, -1.0f);
    }
}

void Game::update(os::Application *application, const float &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Draw characters.
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->clearInstances();
    }
    
    const float baseHeight = terrain->getHeight(vec2(0.0f, 0.0f));
    
    for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
    {
        assert(characterTypes.find(i->second.type) != characterTypes.end());
        characterTypes[i->second.type]->addInstance(i->second, baseHeight);
    }
    
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    //Draw character icons.
    std::vector<draw::WorldIconInstance> iconInstances;
    const float fontHeightScale = 0.2f/fontTexture->getMaxIconDimensions().y;
    
    for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
    {
        if (!i->second.name.empty())
        {
            const vec4 icon = fontTexture->getIcon(i->second.name[0]);
            
            iconInstances.push_back(draw::WorldIconInstance(
                vec3(i->second.position.x, i->second.position.y + baseHeight + characterTypes[i->second.type]->size.y, i->second.position.z),
                vec2(fontHeightScale*icon.z, fontHeightScale*icon.w),
                icon,
                vec4(1.0f, 1.0f, 1.0f, 1.0f)));
        }
    }
    
    fontWorld->setIcons(iconInstances.begin(), iconInstances.end());
    
    //Toggle console.
    if (application->isKeyPressedOnce('`'))
    {
        consoleMode = !consoleMode;
    }
    
    if (application->isKeyPressedOnce('0'))
    {
        Message msg(msg::mt::setPlayerCharacter);

        msg << ownPlayerIndex << 0;
        applyMessage(ownPlayerIndex, msg);
    }
    
    //Direct character selection.
    if (players.find(ownPlayerIndex) != players.end())
    {
        for (int i = 1; i < 10; ++i)
        {
            if (application->isKeyPressedOnce('0' + i) &&
                characters.find(i) != characters.end())
            {
                Message msg(msg::mt::setPlayerCharacter);
    
                msg << ownPlayerIndex << i;
                applyMessage(ownPlayerIndex, msg);
            }
        }
    }
    
    if (consoleMode)
    {
        //Update console input.
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
        //Do we control a character?
        unsigned int characterIndex = 0;
        
        if (players.find(ownPlayerIndex) != players.end())
        {
            characterIndex = players[ownPlayerIndex].characterIndex;
            
            if (characters.find(characterIndex) == characters.end())
            {
                //If we have received a nonzero character id, it should always be valid!
                assert(characterIndex == 0);
                characterIndex = 0;
            }
        }
        
        if (characterIndex == 0)
        {
            //We do not control a character: control the camera directly.
            //Move the camera around and collide with the terrain.
            application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
            cameraPosition.y = std::max(cameraPosition.y, terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f);
        }
        else
        {
            //We do control a character.
            CharacterInstance &chr = characters[characterIndex];
            
            if (application->isKeyPressedOnce('w')) chr.position.x = roundf(chr.position.x + 1.0f);
            if (application->isKeyPressedOnce('s')) chr.position.x = roundf(chr.position.x - 1.0f);
            if (application->isKeyPressedOnce('a')) chr.position.z = roundf(chr.position.z + 1.0f);
            if (application->isKeyPressedOnce('d')) chr.position.z = roundf(chr.position.z - 1.0f);
            if (application->isKeyPressedOnce('q')) chr.position.y = roundf(chr.position.y + 1.0f);
            if (application->isKeyPressedOnce('e')) chr.position.y = std::max(0.0f, roundf(chr.position.y - 1.0f));
            if (application->isKeyPressedOnce('j')) chr.rotation += 2.0f*M_PI/8.0f;
            if (application->isKeyPressedOnce('l')) chr.rotation -= 2.0f*M_PI/8.0f;
        }
        
        //Update the terrain with respect to the camera.
        terrain->terrain->setCameraPosition(cameraPosition);
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
        snd::WorldSounderer::setCamera(cameraPosition, cameraOrientation);
    }
    
    //Update console status.
    updateConsole();
}

void Game::render()
{
    renderer->clearTargets();
    renderer->render();
}

void Game::clear()
{
    //Stops any running game and clears all data.
    
    //Stop networking.
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
    
    //Remove all players except the current user.
    players.clear();
    ownPlayerIndex = 0;
    players.insert(std::make_pair(ownPlayerIndex, Player()));
    
    //Remove all sound sources.
    for (std::map<unsigned int, tiny::snd::Source *>::iterator i = soundSources.begin(); i != soundSources.end(); ++i)
    {
        delete i->second;
    }
    
    soundSources.clear();
    lastSoundSourceIndex = 1;
    
    //Remove all characters.
    characters.clear();
    lastCharacterIndex = 0;
    
    //And add default characters.
    characters[++lastCharacterIndex] = CharacterInstance(0, "Tollie", 0.00f, vec3(0.0f, 0.0f, -1.0f));
    characters[++lastCharacterIndex] = CharacterInstance(0, "Rodan", 0.15f, vec3(-2.0f, 0.0f, -1.0f));
    characters[++lastCharacterIndex] = CharacterInstance(0, "Augustus", 0.30f, vec3(-1.0f, 0.0f, 0.0f));
    characters[++lastCharacterIndex] = CharacterInstance(0, "Sven", 0.45f, vec3(0.0f, 0.0f, 1.0f));
    characters[++lastCharacterIndex] = CharacterInstance(0, "Julius", 0.60f, vec3(0.0f, 0.0f, 0.0f));
    characters[++lastCharacterIndex] = CharacterInstance(0, "Laertes", 0.75f, vec3(-2.0f, 0.0f, 1.0f));
    
    //Reset camera.
    cameraPosition = vec3(0.0f, 0.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

GameMessageTranslator *Game::getTranslator() const
{
    return translator;
}

void Game::readResources(const std::string &path)
{
    const std::string worldFileName = path + "rpg.xml";
    
    std::cerr << "Reading resources from '" << worldFileName << "'." << std::endl;
    
    //Read world description from an XML file.
    TiXmlDocument doc(worldFileName.c_str());
    
    if (!doc.LoadFile())
    {
        std::cerr << "Unable to read XML file '" << worldFileName << "'!" << std::endl;
        throw std::exception();
    }
    
    TiXmlElement *root = doc.RootElement();
    
    if (!root)
    {
        std::cerr << "No XML root node!" << std::endl;
        throw std::exception();
    }
    
    if (std::string(root->Value()) != "rpg")
    {
        std::cerr << "This is not a valid RPG XML file!" << std::endl;
        throw std::exception();
    }
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
             if (std::string(el->Value()) == "console") readConsoleResources(path, el);
        else if (std::string(el->Value()) == "sky") readSkyResources(path, el);
        else if (std::string(el->Value()) == "terrain") terrain = new GameTerrain(path, el);
        else if (std::string(el->Value()) == "charactertype") characterTypes[characterTypes.size()] = new CharacterType(path, el);
        else std::cerr << "Warning: unknown data " << el->Value() << " encountered in XML!" << std::endl;
    }
}

void Game::readConsoleResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading console resources..." << std::endl;
    
    int fontTextureSize = 0;
    int fontSize = 0;
    std::string fontFileName = "";
    
    assert(std::string(el->Value()) == "console");
    
    el->QueryIntAttribute("texture_size", &fontTextureSize);
    el->QueryIntAttribute("font_size", &fontSize);
    el->QueryStringAttribute("font", &fontFileName);
    
    //Read font from disk as a texture.
    fontTexture = new draw::IconTexture2D(fontTextureSize, fontTextureSize);
    fontTexture->packIcons(img::io::readFont(path + fontFileName, fontSize));
    
    //Create a drawable font object as a collection of instanced screen icons.
    font = new draw::ScreenIconHorde(4096);
    font->setIconTexture(*fontTexture);
    font->setText(-1.0, -1.0, 0.1, aspectRatio, "", *fontTexture);
    consoleBackground = new draw::effects::Solid();
    
    //Create font to put text inside the world.
    fontWorld = new draw::WorldIconHorde(4096, true);
    fontWorld->setIconTexture(*fontTexture);
}

void Game::readSkyResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading sky resources..." << std::endl;
    
    std::string textureFileName = "";
    
    assert(std::string(el->Value()) == "sky");
    
    el->QueryStringAttribute("texture", &textureFileName);
    
    //Create sky box mesh and read gradient texture.
    skyBoxMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
    skyBoxDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    skyBoxMesh->setDiffuseTexture(*skyBoxDiffuseTexture);
    
    skyEffect = new draw::effects::SunSky();
    skyGradientTexture = new draw::RGBTexture2D(img::io::readImage(path + textureFileName), draw::tf::filter);
    skyEffect->setSkyTexture(*skyGradientTexture);
    skyEffect->setSun(normalize(vec3(0.1f, 1.0f, 0.2f)));
}

