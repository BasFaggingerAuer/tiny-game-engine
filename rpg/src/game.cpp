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
        renderer->addWorldRenderable(index++, i->second->shadowHorde);
    }
    
    renderer->addWorldRenderable(index++, voxelMap->voxelMap);
    
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
    delete fontWorld;
    delete fontTexture;
    
    delete voxelMap;
    
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
        
        unsigned int characterIndex = 0;
        
        if (players.find(ownPlayerIndex) != players.end())
        {
            characterIndex = players.at(ownPlayerIndex).characterIndex;
        }
        
        for (auto i = characters.begin(); i != characters.end(); ++i)
        {
            const vec4 c = i->second.getColor();
            
            strm << "\\#fff" << i->first << (i->first == characterIndex ? "*(" : " (") << std::dec << std::setprecision(0) << std::fixed << 5*i->second.position.x << ", " << 5*i->second.position.y << ", " << 5*i->second.position.z << "): \\#" << std::hex << std::min(15, static_cast<int>(16.0f*c.x)) << std::min(15, static_cast<int>(16.0f*c.y)) << std::min(15, static_cast<int>(16.0f*c.z)) << i->second.name << std::endl;
        }
        
        if (characterIndex == 0)
        {
            strm << std::endl << "\\#fff";
            strm << " Z/C = Previous/next character" << std::endl;
            strm << (paintMode == VoxelReplace ? "*" : " ") << "V = Replace/Pick voxels" << std::endl;
            strm << (paintMode == VoxelAdd ? "*" : " ") << "B = Add/Remove voxels" << std::endl;
            strm << " P = Restore voxel palette" << std::endl;
            strm << " \\\\ = Flood base plane" << std::endl;
        }
        else
        {
            strm << std::endl << "\\#fff";
            strm << " 0: Return to voxel edit mode" << std::endl;
        }
        
        font->setText(-1.0, -1.0, 0.05, aspectRatio, strm.str(), *fontTexture);
        consoleBackground->setColour(vec4(0.0f, 0.0f, 0.0f, 0.3f));
        consoleBackground->setSquareDimensions(-1.0f, 1.0f, -0.6f, -1.0f);
    }
}

void Game::update(os::Application *application, const float &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Draw characters and their icons.
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->clearInstances();
    }
    
    std::vector<draw::WorldIconInstance> iconInstances;
    const float fontHeightScale = 0.15f/fontTexture->getMaxIconDimensions().y;
    
    for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
    {
        assert(characterTypes.find(i->second.type) != characterTypes.end());
        
        const int baseHeight = voxelMap->getBaseHeight(i->second.position.x, i->second.position.z);
        
        characterTypes[i->second.type]->addInstance(i->second, baseHeight);
        
        if (!i->second.name.empty())
        {
            const vec4 icon = fontTexture->getIcon(i->second.name[0]);
            
            iconInstances.push_back(draw::WorldIconInstance(
                vec3(static_cast<float>(i->second.position.x) + 0.5f,
                     static_cast<float>(i->second.position.y + baseHeight) + 0.5f*characterTypes[i->second.type]->size.y,
                     static_cast<float>(i->second.position.z) + 0.5f),
                vec2(fontHeightScale*icon.z, fontHeightScale*icon.w),
                icon,
                vec4(1.0f, 1.0f, 1.0f, 1.0f)));
        }
    }
    
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    fontWorld->setIcons(iconInstances.begin(), iconInstances.end());
    
    //Update mouse timer.
    if (mouseTimer > 0.0f)
    {
        mouseTimer -= dt;
    }
    
    //Toggle console.
    if (application->isKeyPressedOnce('`'))
    {
        consoleMode = !consoleMode;
        
        if (consoleMode) application->getTextInput();
    }
    
    if (consoleMode)
    {
        //Update console input.
        const std::string text = application->getTextInput();
        
        for (auto j = text.cbegin(); j != text.cend(); ++j)
        {
            if (*j != '`' && *j != '\b' && *j != '\n' && *j != '\r' && *j != '[' && *j != ']') console->keyDown(*j);
        }
        
        if (application->isKeyPressedOnce('\b')) console->keyDown('\b');
        if (application->isKeyPressedOnce('\n')) console->keyDown('\n');
        if (application->isKeyPressedOnce('\r')) console->keyDown('\r');
        if (application->isKeyPressedOnce('[')) console->scrollUp();
        if (application->isKeyPressedOnce(']')) console->scrollDown();
    }
    else
    {
        //Release control of characters.
        if (application->isKeyPressedOnce('0') || application->isKeyPressedOnce(' '))
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
            
            if (application->isKeyPressedOnce('z') &&
                players[ownPlayerIndex].characterIndex > 1)
            {
                Message msg(msg::mt::setPlayerCharacter);
    
                msg << ownPlayerIndex << players[ownPlayerIndex].characterIndex - 1;
                applyMessage(ownPlayerIndex, msg);
            }
            
            if (application->isKeyPressedOnce('c'))
            {
                Message msg(msg::mt::setPlayerCharacter);
    
                msg << ownPlayerIndex << players[ownPlayerIndex].characterIndex + 1;
                applyMessage(ownPlayerIndex, msg);
            }
        }
    
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
            //We do not control a character.
            
            //Change edit mode.
            if (application->isKeyPressedOnce('v'))
            {
                paintMode = VoxelReplace;
            }
            if (application->isKeyPressedOnce('b'))
            {
                paintMode = VoxelAdd;
            }
            if (application->isKeyPressedOnce('p'))
            {
                voxelMap->createVoxelPalette();
            }
            if (application->isKeyPressedOnce('\\'))
            {
                voxelMap->setVoxelBasePlane(paintVoxelType);
            }
            
            //Did the user click anywhere?
            auto mouse = application->getMouseState(false);
            
            if (mouse.buttons != 0)
            {
                if (mouseTimer <= 0.0f || application->isKeyPressed('f'))
                {
                    mouseTimer = 0.25f;
                    
                    //Check which voxel we hit.
                    auto voxelHit = voxelMap->getIntersection(cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));
                    
                    if (voxelHit.distance > 0.0f)
                    {
                        if (mouse.buttons == 4)
                        {
                            //Find closest character.
                            const ivec3 p = voxelHit.voxelIndices;
                            int foundCharacter = -1;
                            int minDistanceSq = 5*5;
                            
                            for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
                            {
                                const ivec3 d = i->second.position - p;
                                const int distanceSq = d.x*d.x + d.z*d.z;
                                
                                if (distanceSq < minDistanceSq)
                                {
                                    foundCharacter = i->first;
                                    minDistanceSq = distanceSq;
                                }
                            }
                            
                            if (foundCharacter >= 0)
                            {
                                Message msg(msg::mt::setPlayerCharacter);
                                
                                msg << ownPlayerIndex << foundCharacter;
                                applyMessage(ownPlayerIndex, msg);
                            }
                        }
                        else
                        {
                            switch (paintMode)
                            {
                                case VoxelReplace:
                                    if (mouse.buttons == 1)
                                    {
                                        voxelMap->setVoxel(voxelHit.voxelIndices, paintVoxelType);
                                    }
                                    else if (mouse.buttons == 2)
                                    {
                                        paintVoxelType = voxelMap->getVoxel(voxelHit.voxelIndices);
                                    }
                                    break;
                                case VoxelAdd:
                                    if (mouse.buttons == 1)
                                    {
                                        voxelMap->setVoxel(voxelHit.voxelIndices + voxelHit.normal, paintVoxelType);
                                    }
                                    else if (mouse.buttons == 2)
                                    {
                                        voxelMap->setVoxel(voxelHit.voxelIndices, 0);
                                    }
                                    break;
                            }
                        }
                    }
                }
            }
            else
            {
                mouseTimer = 0.0f;
            }
            
            //Move the camera around and collide with the terrain.
            const float cameraRadius = 1.0f;
            vec3 newPosition = cameraPosition;
            
            application->updateSimpleCamera(dt, newPosition, cameraOrientation);
            newPosition.y = std::max(newPosition.y, terrain->getHeight(vec2(newPosition.x, newPosition.z)) + cameraRadius);
            newPosition.x = clamp(newPosition.x, -0.5f*voxelMap->getScaledWidth(), 0.5f*voxelMap->getScaledWidth());
            newPosition.y = clamp(newPosition.y, voxelMap->getScale() + cameraRadius, voxelMap->getScaledHeight());
            newPosition.z = clamp(newPosition.z, -0.5f*voxelMap->getScaledDepth(), 0.5f*voxelMap->getScaledDepth());
            cameraPosition = newPosition;
        }
        else
        {
            //We do control a character.
            CharacterInstance &chr = characters[characterIndex];
            
            if (application->isKeyPressedOnce('s')) chr.position.x = chr.position.x + 1;
            if (application->isKeyPressedOnce('w')) chr.position.x = chr.position.x - 1;
            if (application->isKeyPressedOnce('a')) chr.position.z = chr.position.z + 1;
            if (application->isKeyPressedOnce('d')) chr.position.z = chr.position.z - 1;
            if (application->isKeyPressedOnce('q')) chr.position.y = chr.position.y + 1;
            if (application->isKeyPressedOnce('e')) chr.position.y = std::max(0, chr.position.y - 1);
            if (application->isKeyPressedOnce('j')) chr.rotation += 45;
            if (application->isKeyPressedOnce('l')) chr.rotation -= 45;
            
            //Did the user click anywhere?
            auto mouse = application->getMouseState(false);
            
            if (mouse.buttons != 0)
            {
                if (mouseTimer <= 0.0f || application->isKeyPressed('f'))
                {
                    mouseTimer = 0.25f;
                    
                    //Check which voxel we hit.
                    auto voxelHit = voxelMap->getIntersection(cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));
                    
                    if (voxelHit.distance > 0.0f)
                    {
                        if (mouse.buttons == 1)
                        {
                            chr.position.x = voxelHit.voxelIndices.x;
                            chr.position.z = voxelHit.voxelIndices.z;
                        }
                    }
                }
            }
            else
            {
                mouseTimer = 0.0f;
            }
            
            //Is this a network game?
            if (host || client)
            {
                Message msg(msg::mt::updateCharacter);
                
                msg << characterIndex << chr.position << chr.rotation << chr.color;
                
                if (client) client->sendMessage(msg);
                if (host) host->sendMessage(msg);
            }
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
    
    //Reset all characters.
    lastCharacterIndex = 0;
    
    for (auto i = baseCharacters.cbegin(); i != baseCharacters.end(); ++i)
    {
        characters[++lastCharacterIndex] = *i;
    }
    
    //Reset camera.
    cameraPosition = vec3(0.0f, 10.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    paintMode = VoxelReplace;
    paintVoxelType = 1;
    mouseTimer = 0.0f;
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
        else if (std::string(el->Value()) == "voxelmap") voxelMap = new GameVoxelMap(path, el);
        else if (std::string(el->Value()) == "terrain") terrain = new GameTerrain(path, el);
        else if (std::string(el->Value()) == "charactertype") characterTypes[characterTypes.size()] = new CharacterType(path, el);
        else if (std::string(el->Value()) == "character") readCharacterResources(el);
        else std::cerr << "Warning: unknown data " << el->Value() << " encountered in XML!" << std::endl;
    }
    
    //Check that all renderables have been created.
    if (!font || !skyEffect || !terrain || !voxelMap)
    {
        std::cerr << "Error: Not all resources were initialized (incomplete XML?)!" << std::endl;
        throw std::exception();
    }
    
    //Assign voxel map to the sky effect.
    skyEffect->setVoxelMap(*(voxelMap->voxelTexture), voxelMap->getScale());
}

void Game::readCharacterResources(TiXmlElement *el)
{
    std::cerr << "Reading character resources..." << std::endl;
    
    assert(std::string(el->Value()) == "character");
    
    std::string name = "";
    std::string type = "";
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float r = 0.0f;
    float c = 0.0f;
    
    el->QueryStringAttribute("name", &name);
    el->QueryStringAttribute("type", &type);
    el->QueryFloatAttribute("x", &x);
    el->QueryFloatAttribute("y", &y);
    el->QueryFloatAttribute("z", &z);
    el->QueryFloatAttribute("r", &r);
    el->QueryFloatAttribute("c", &c);
    
    unsigned int typeIndex = 0;
    bool typeFound = false;
    
    for (auto i = characterTypes.cbegin(); i != characterTypes.cend() && !typeFound; ++i)
    {
        if (i->second->name == type)
        {
            typeFound = true;
            typeIndex = i->first;
        }
    }
    
    if (!typeFound)
    {
        std::cerr << "Unable to find character type '" << type << "' for character '" << name << "'!" << std::endl;
        throw std::exception();
    }
    
    baseCharacters.push_back(CharacterInstance(typeIndex, name, ivec3(x, y, z), r, c));
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
    int nrSteps = 128;
    
    assert(std::string(el->Value()) == "sky");
    
    el->QueryStringAttribute("texture", &textureFileName);
    el->QueryIntAttribute("nr_steps", &nrSteps);
    
    //Create sky box mesh and read gradient texture.
    skyBoxMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
    skyBoxDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    skyBoxMesh->setDiffuseTexture(*skyBoxDiffuseTexture);
    
    skyEffect = new draw::effects::SunSkyVoxelMap(nrSteps);
    skyGradientTexture = new draw::RGBTexture2D(img::io::readImage(path + textureFileName), draw::tf::filter);
    skyEffect->setSkyTexture(*skyGradientTexture);
    skyEffect->setSun(normalize(vec3(0.4f, 0.8f, 0.4f)));
}

