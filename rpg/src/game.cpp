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

#include <cstdio>
#include <cstdlib>
#include <ctime>
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
    aspectRatio(static_cast<float>(application->getScreenWidth())/static_cast<float>(application->getScreenHeight())),
    mouseSensitivity(64.0f),
    mouseLook(true),
    lastCharacterIndex(1),
    selectedCharacterType(0),
    selectedCharacterColor(0.0f),
    translator(new GameMessageTranslator()),
    console(new GameConsole(this)),
    host(0),
    client(0),
    ownPlayerIndex(0),
    nrVoxelMapChunks(0),
    voxelMapChunks()
{
    readResources(path);
    consoleMode = false;
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    renderTimesInNs.clear();
    renderNrFrames = 0;
    
    unsigned int index = 0;
    
    renderer->addWorldRenderable(index++, terrain->terrain);
    
    for (std::map<unsigned int, CharacterType *>::const_iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        renderer->addWorldRenderable(index++, i->second->horde);
        renderer->addWorldRenderable(index++, i->second->shadowHorde);
    }
    
    renderer->addWorldRenderable(index++, voxelMap->voxelMap);
    renderer->addWorldRenderable(index++, skyBoxMesh);
    
    renderer->addScreenRenderable(index++, skyEffect, false, false);
    renderer->addScreenRenderable(index++, fontWorld, false, false, draw::BlendMode::BlendMix);
    renderer->addScreenRenderable(index++, consoleBackground, false, false, draw::BlendMode::BlendMix);
    renderer->addScreenRenderable(index++, font, false, false, draw::BlendMode::BlendMix);
    
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
        font->setText(-1.0f, -1.0f, 0.05f, aspectRatio, console->getText(256), *fontTexture);
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
        
        /*
        for (auto i = characters.begin(); i != characters.end(); ++i)
        {
            const vec4 c = i->second.getColor();
            
            strm << "\\#fff" << std::dec << i->first << (i->first == characterIndex ? "*(" : " (") << std::dec << std::setprecision(0) << std::fixed << 5*i->second.position.x << ", " << 5*i->second.position.y << ", " << 5*i->second.position.z << "): \\#" << std::hex << std::min(15, static_cast<int>(16.0f*c.x)) << std::min(15, static_cast<int>(16.0f*c.y)) << std::min(15, static_cast<int>(16.0f*c.z)) << i->second.name << std::endl;
        }
        */
        
        if (characterIndex == 0)
        {
            strm << "\\#fff";
            strm << " Z = Export game state" << std::endl;
            strm << (paintMode == PaintMode::Character ? "*" : " ") << "C = Create/delete character" << std::endl;
            strm << (paintMode == PaintMode::VoxelReplace ? "*" : " ") << "V = Replace/Pick voxels" << std::endl;
            strm << (paintMode == PaintMode::VoxelAdd ? "*" : " ") << "B = Add/Remove voxels" << std::endl;
            strm << " P = Restore voxel palette" << std::endl;
            strm << " \\\\ = Flood base plane" << std::endl;
            
            const vec4 c = CharacterInstance::palette(selectedCharacterColor);

            strm << " 0-9 = Select " << draw::ScreenIconHorde::getColorCode(c.x, c.y, c.z) << "color" << std::endl << "\\#fff";
            strm << " WASDQE = Move" << std::endl;
            strm << " IJKLOU = Look" << std::endl;
            strm << " F = Fast mode" << std::endl;
            strm << " G = Block replace/add voxels" << std::endl;
            strm << " Mouse whl. = Select char." << std::endl;
            strm << " M/. = Select character type" << std::endl;
            
            auto ct = characterTypes.find(selectedCharacterType);
            
            if (ct != characterTypes.end())
            {
                strm << "       " << ct->second->name << std::endl;
            }
        }
        else
        {
            strm << std::endl << "\\#fff";
            strm << " <Space>: Return to voxel edit mode" << std::endl;
            strm << " WASDQE = Move" << std::endl;
            strm << " F = Knock prone" << std::endl;
            strm << " G = Enlarge" << std::endl;
        }
        
        strm << "Controls:" << std::endl;
        
        font->setText(-1.0f, -1.0f, 0.05f, aspectRatio, strm.str(), *fontTexture);
        consoleBackground->setColour(vec4(0.0f, 0.0f, 0.0f, 0.3f));
        consoleBackground->setSquareDimensions(-1.0f, -0.30f, -0.55f, -1.0f);
    }
}

void Game::update(os::Application *application, const float &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Draw characters and their icons.
    for (auto i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->clearInstances();
    }
    
    std::vector<draw::WorldIconInstance> iconInstances;
    const float fontHeightScale = 0.15f/fontTexture->getMaxIconDimensions().y;
    
    for (auto i = characters.cbegin(); i != characters.cend(); ++i)
    {
        assert(characterTypes.find(i->second.type) != characterTypes.end());
        
        const int baseHeight = voxelMap->getBaseHeight(i->second.position.x, i->second.position.z);
        
        characterTypes[i->second.type]->addInstance(i->second, static_cast<float>(baseHeight));
        
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
    
    for (auto i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        if (i->first == selectedCharacterType && paintMode == PaintMode::Character)
        {
            //Show selected character type.
            auto mouse = application->getMouseState(false);
            auto voxelHit = voxelMap->getIntersection(cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));
            
            if (voxelHit.distance > 0.0f)
            {
                i->second->addInstance(CharacterInstance(i->first, "", voxelHit.voxelIndices, 0, 0, selectedCharacterColor), 1.0f);
            }
        }

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
        if (application->isKeyPressedOnce(' '))
        {
            Message msg(msg::mt::setPlayerCharacter);

            msg << ownPlayerIndex << 0;
            applyMessage(ownPlayerIndex, msg);
        }
        
        //Select character color.
        if (application->isKeyPressedOnce('0')) selectedCharacterColor = -1.0f;
        if (application->isKeyPressedOnce('9')) selectedCharacterColor = -0.75f;
        if (application->isKeyPressedOnce('1')) selectedCharacterColor =  0.125f;
        if (application->isKeyPressedOnce('2')) selectedCharacterColor =  0.250f;
        if (application->isKeyPressedOnce('3')) selectedCharacterColor =  0.375f;
        if (application->isKeyPressedOnce('4')) selectedCharacterColor =  0.500f;
        if (application->isKeyPressedOnce('5')) selectedCharacterColor =  0.625f;
        if (application->isKeyPressedOnce('6')) selectedCharacterColor =  0.750f;
        if (application->isKeyPressedOnce('7')) selectedCharacterColor =  0.875f;
        if (application->isKeyPressedOnce('8')) selectedCharacterColor =  1.000f;

        //Select character type.
        if (application->isKeyPressedOnce('m'))
        {
            if (selectedCharacterType > 0) --selectedCharacterType;
        }
            
        if (application->isKeyPressedOnce('.'))
        {
            auto m = std::max_element(characterTypes.cbegin(), characterTypes.cend(),
                                        [] (const std::pair<unsigned int, CharacterType *> &lhs, const std::pair<unsigned int, CharacterType *> &rhs) {return lhs.first < rhs.first;});

            if (m != characterTypes.cend())
            {
                selectedCharacterType = std::min<unsigned int>(selectedCharacterType + 1, m->first);
            }
            else
            {
                selectedCharacterType = 0;
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
            if (mouseLook) {
                if (application->isKeyPressed('c')) paintMode = PaintMode::Character;
                else if (application->isKeyPressed('v')) paintMode = PaintMode::VoxelReplace;
                else if (application->isKeyPressed('b')) paintMode = PaintMode::VoxelAdd;
                else paintMode = PaintMode::None;
            }
            else {
                if (application->isKeyPressedOnce('c'))
                {
                    paintMode = PaintMode::Character;
                    startedVoxelSelection = false;
                }
                if (application->isKeyPressedOnce('v'))
                {
                    paintMode = PaintMode::VoxelReplace;
                    startedVoxelSelection = false;
                }
                if (application->isKeyPressedOnce('b'))
                {
                    paintMode = PaintMode::VoxelAdd;
                    startedVoxelSelection = false;
                }
                if (application->isKeyPressedOnce('n'))
                {
                    paintMode = PaintMode::None;
                    startedVoxelSelection = false;
                }
            }

            if (application->isKeyPressedOnce('p'))
            {
                voxelMap->createVoxelPalette();
            }
            if (application->isKeyPressedOnce('\\'))
            {
                Message msg(msg::mt::updateVoxelBasePlane);
                
                msg << paintVoxelType;
                applyMessage(ownPlayerIndex, msg);
            }
            if (application->isKeyPressedOnce('z'))
            {
                //Export current map state to XML-friendly format.
                const std::string levelFile = "level.xml";
                
                std::cerr << "Appending current map state to '" << levelFile << "'..." << std::endl;
                
                std::ofstream of(levelFile.c_str(), std::ios::app | std::ios::out);
                std::time_t currentTime = std::time(nullptr);
                
                of << "Level export " << std::put_time(std::localtime(&currentTime), "%F %T") << ":" << std::endl;
                
                for (auto ch = characters.cbegin(); ch != characters.cend(); ++ch)
                {
                    Message msg(msg::mt::addCharacter);
                    std::string str = "";

                    msg << ch->first << ch->second.name << ch->second.type << ch->second.color;
                    translator->messageToText(msg, str);
                    of << "<init command=\"" << str << "\" />" << std::endl;
                }

                for (auto ch = characters.cbegin(); ch != characters.cend(); ++ch)
                {
                    Message msg(msg::mt::updateCharacter);
                    std::string str = "";

                    msg << ch->first << ch->second.position << ch->second.rotation << ch->second.state << ch->second.color;
                    translator->messageToText(msg, str);
                    of << "<init command=\"" << str << "\" />" << std::endl;
                }

                if (true)
                {
                    const std::string text = voxelMap->getCompressedVoxels();
                    const int chunkSize = 255;
                    int nrChunks = (static_cast<int>(text.size())/chunkSize) + (static_cast<int>(text.size()) % chunkSize == 0 ? 0 : 1);
                    std::string str = "";

                    Message msgStart(msg::mt::startVoxelMap);

                    msgStart << nrChunks;
                    translator->messageToText(msgStart, str);
                    of << "<init command=\"" << str << "\" />" << std::endl;
                    
                    for (int ck = 0; ck < nrChunks; ++ck)
                    {
                        Message msg(msg::mt::chunkVoxelMap);
                        
                        msg << text.substr(chunkSize*ck, chunkSize);
                        translator->messageToText(msg, str);
                        of << "<init command=\"" << str << "\" />" << std::endl;
                    }
                }
                of << "End of export." << std::endl << std::endl;
                of.close();
                std::cerr << "Export completed." << std::endl;
            }
            /*
            if (application->isKeyPressedOnce('x'))
            {
                draw::VoxelMap::setDistanceMap(*(voxelMap->voxelTexture));
            }
            */

            auto mouse = application->getMouseState(false);
            
            //Do we want to block-fill?
            if (application->isKeyPressedOnce('g'))
            {
                //Check which voxel we hit.
                auto voxelHit = voxelMap->getIntersection(cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));

                if (voxelHit.distance > 0.0f)
                {
                    if (!startedVoxelSelection)
                    {
                        startedVoxelSelection = true;
                        voxelSelectionStart = voxelHit.voxelIndices;
                    }
                    else if (paintMode == PaintMode::VoxelReplace || paintMode == PaintMode::VoxelAdd)
                    {
                        const tiny::ivec3 v0 = tiny::min(voxelSelectionStart, voxelHit.voxelIndices);
                        const tiny::ivec3 v1 = tiny::max(voxelSelectionStart, voxelHit.voxelIndices);
                        const tiny::ivec3 dv = (paintMode == PaintMode::VoxelReplace ? tiny::ivec3(0, 0, 0) : voxelHit.normal);

                        for (auto vx = v0.x; vx <= v1.x; ++vx)
                        {
                            for (auto vz = v0.z; vz <= v1.z; ++vz)
                            {
                                Message msg(msg::mt::updateVoxel);
                                    
                                msg << tiny::ivec3(vx, v0.y, vz) + dv << paintVoxelType;
                                                
                                if (client) client->sendMessage(msg);
                                else applyMessage(ownPlayerIndex, msg);
                            }
                        }

                        startedVoxelSelection = false;
                    }
                }
                else
                {
                    startedVoxelSelection = false;
                }
            }

            //Did the user click anywhere?
            if (mouse.buttons != 0)
            {
                if (mouseTimer <= 0.0f || application->isKeyPressed('f'))
                {
                    mouseTimer = 0.25f;
                    
                    //Check which voxel we hit.
                    auto voxelHit = voxelMap->getIntersection(cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));
                    
                    if (voxelHit.distance > 0.0f)
                    {
                        //Find closest character.
                        //Find closest character.
                        int foundCharacter = -1;
                        int characterDistanceSq = 32*32;

                        if (true)
                        {
                            const ivec3 p = voxelHit.voxelIndices;
                        
                            for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
                            {
                                const ivec3 d = i->second.position - p;
                                const int distanceSq = d.x*d.x + d.z*d.z;
                                        
                                if (distanceSq < characterDistanceSq)
                                {
                                    foundCharacter = i->first;
                                    characterDistanceSq = distanceSq;
                                }
                            }
                        }

                        //Select characters with the middle-mouse button.
                        if (mouse.buttons == 4)
                        {
                            if (foundCharacter >= 0)
                            {
                                Message msg(msg::mt::setPlayerCharacter);
                                        
                                msg << ownPlayerIndex << foundCharacter;
                                
                                if (client) client->sendMessage(msg);
                                else applyMessage(ownPlayerIndex, msg);
                            }
                        }

                        switch (paintMode)
                        {
                            case PaintMode::None:
                                break;
                            case PaintMode::Character:
                                if (mouse.buttons == 1)
                                {
                                    //Create character (only if there is no character yet in this square).
                                    if (foundCharacter < 0 || characterDistanceSq > 0)
                                    {
                                        const ivec3 p = voxelHit.voxelIndices;
                                        
                                        auto ct = characterTypes.find(selectedCharacterType);
                
                                        if (ct != characterTypes.end())
                                        {
                                            if (true)
                                            {
                                                Message msg(msg::mt::addCharacter);
                                                
                                                msg << ++lastCharacterIndex << ct->second->name << ct->first << selectedCharacterColor;

                                                if (client) client->sendMessage(msg);
                                                else applyMessage(ownPlayerIndex, msg);
                                            }
                                            
                                            if (true)
                                            {
                                                Message msg(msg::mt::updateCharacter);
                                            
                                                msg << lastCharacterIndex << ivec3(p.x, 0, p.z) << 0 << 0 << selectedCharacterColor;
                                                
                                                if (client) client->sendMessage(msg);
                                                else applyMessage(ownPlayerIndex, msg);
                                            }
                                        }
                                    }
                                }
                                else if (mouse.buttons == 2)
                                {
                                    //Delete character.
                                    if (foundCharacter >= 0 && characterDistanceSq <= 1)
                                    {
                                        Message msg(msg::mt::removeCharacter);
                                        
                                        msg << foundCharacter;
                                        
                                        if (client) client->sendMessage(msg);
                                        else applyMessage(ownPlayerIndex, msg);
                                    }
                                }
                                break;
                            case PaintMode::VoxelReplace:
                                if (mouse.buttons == 1)
                                {
                                    Message msg(msg::mt::updateVoxel);
                                    
                                    msg << voxelHit.voxelIndices << paintVoxelType;
                                    
                                    if (client) client->sendMessage(msg);
                                    else applyMessage(ownPlayerIndex, msg);
                                }
                                else if (mouse.buttons == 2)
                                {
                                    paintVoxelType = voxelMap->getVoxel(voxelHit.voxelIndices);
                                }
                                break;
                            case PaintMode::VoxelAdd:
                                if (mouse.buttons == 1)
                                {
                                    Message msg(msg::mt::updateVoxel);
                                    
                                    msg << voxelHit.voxelIndices + voxelHit.normal << paintVoxelType;
                                    
                                    if (client) client->sendMessage(msg);
                                    else applyMessage(ownPlayerIndex, msg);
                                }
                                else if (mouse.buttons == 2)
                                {
                                    Message msg(msg::mt::updateVoxel);
                                    
                                    msg << voxelHit.voxelIndices << 0;
                                    
                                    if (client) client->sendMessage(msg);
                                    else applyMessage(ownPlayerIndex, msg);
                                }
                                break;
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

            if (paintMode == PaintMode::None && mouseLook)
            {
                const tiny::os::MouseState mouseState = application->getMouseState(true);
                const vec2 mouseDelta = mouseSensitivity*dt*vec2(mouseState.x, mouseState.y);
                
                cameraOrientation = normalize(quatmul(quatrot(mouseDelta.x, tiny::vec3(0.0f, 1.0f, 0.0f)), 
                                                quatmul(quatrot(mouseDelta.y, tiny::vec3(1.0f, 0.0f, 0.0f)), 
                                                    cameraOrientation)));
            }

            newPosition.y = std::max(newPosition.y, terrain->getHeight(vec2(newPosition.x, newPosition.z)) + cameraRadius);
            newPosition.x = clamp(newPosition.x, -0.5f*voxelMap->getScaledWidth(), 0.5f*voxelMap->getScaledWidth() - voxelMap->getScale());
            newPosition.y = clamp(newPosition.y, voxelMap->getScale() + cameraRadius, voxelMap->getScaledHeight() - voxelMap->getScale());
            newPosition.z = clamp(newPosition.z, -0.5f*voxelMap->getScaledDepth(), 0.5f*voxelMap->getScaledDepth() - voxelMap->getScale());
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
            if (application->isKeyPressedOnce('f')) chr.state ^= 1;
            if (application->isKeyPressedOnce('g')) chr.state ^= 2;
            
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
                
                msg << characterIndex << chr.position << chr.rotation << chr.state << chr.color;
                
                if (client) client->sendMessage(msg);
                if (host) host->sendMessage(msg);
            }
        }
        
        //Update the terrain with respect to the camera.
        terrain->terrain->setCameraPosition(cameraPosition);
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
    }
    
    //Update console status.
    updateConsole();

    //Update screen size in case it has changed.
    renderer->setScreenSize(application->getScreenWidth(), application->getScreenHeight());
}

void Game::render()
{
    renderer->clearTargets();
    auto deltaTimes = renderer->render();

    if (!deltaTimes.empty())
    {
        if (deltaTimes.size() != renderTimesInNs.size())
        {
            renderTimesInNs = deltaTimes;
            renderNrFrames = 1;
        }
        else
        {
            for (size_t i = 0; i < renderTimesInNs.size(); ++i)
            {
                assert(renderTimesInNs[i].first == deltaTimes[i].first);
                renderTimesInNs[i].second += deltaTimes[i].second;
            }

            if (++renderNrFrames >= 240)
            {
                std::cerr << "Render times in (ms) for " << renderNrFrames << " frames:" << std::endl;

                for (auto i = renderTimesInNs.cbegin(); i != renderTimesInNs.cend(); ++i)
                {
                    std::cerr << "    " << std::setw(16) << i->first << ": " << std::fixed << std::setw(1) << std::setprecision(4) << 1.0e-6*static_cast<double>(i->second)/static_cast<double>(renderNrFrames) << std::endl;
                }

                renderTimesInNs = deltaTimes;
                renderNrFrames = 1;
            }
        }
    }
}

void Game::clear()
{
    //Stops any running game and clears all data.
    console->addLine("Resetting game state...");
    
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
    
    //Reset performance timers.
    renderTimesInNs.clear();

    //Remove all players except the current user.
    players.clear();
    ownPlayerIndex = 0;
    players.insert(std::make_pair(ownPlayerIndex, Player()));
    
    //Reset all characters.
    lastCharacterIndex = 0;
    selectedCharacterType = 0;
    selectedCharacterColor = 0.0f;
    characters.clear();
    
    //Reset camera.
    cameraPosition = vec3(0.0f, 10.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    paintMode = PaintMode::None;
    paintVoxelType = 1;
    startedVoxelSelection = false;
    voxelSelectionStart = tiny::ivec3(0, 0, 0);
    mouseTimer = 0.0f;

    //Reset voxel map.
    voxelMap->setVoxelBasePlane(1);
    voxelMap->createVoxelPalette();

    nrVoxelMapChunks = 0;
    voxelMapChunks.clear();
    
    //Run initialization commands.
    console->addLine("Running initialization...");
    
    for (auto i = initCommands.cbegin(); i != initCommands.cend(); ++i)
    {
        for (auto j = i->cbegin(); j != i->cend(); ++j)
        {
            console->keyDown(*j);
        }
        
        console->keyDown('\n');
    }
    
    console->addLine("Reset and initialization complete.");
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
    
    int haveMouseLook = 1;
    
    root->QueryIntAttribute("mouse_look", &haveMouseLook);
    mouseLook = (haveMouseLook != 0);
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
             if (std::string(el->Value()) == "console") readConsoleResources(path, el);
        else if (std::string(el->Value()) == "sky") readSkyResources(path, el);
        else if (std::string(el->Value()) == "voxelmap") voxelMap = new GameVoxelMap(path, el);
        else if (std::string(el->Value()) == "terrain") terrain = new GameTerrain(path, el);
        else if (std::string(el->Value()) == "charactertype") characterTypes[characterTypes.size() + 1] = new CharacterType(path, el);
        else if (std::string(el->Value()) == "init")
        {
            std::string command = "";
            
            el->QueryStringAttribute("command", &command);
            
            if (!command.empty())
            {
                initCommands.push_back(command);
            }
        }
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
    font->setText(-1.0f, -1.0f, 0.1f, aspectRatio, "", *fontTexture);
    consoleBackground = new draw::effects::Solid();
    
    //Create font to put text inside the world.
    fontWorld = new draw::WorldIconHorde(4096, true);
    fontWorld->setIconTexture(*fontTexture);
}

void Game::readSkyResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading sky resources..." << std::endl;
    
    std::string textureFileName = "";
    int nrSteps = 1;
    
    assert(std::string(el->Value()) == "sky");
    
    el->QueryStringAttribute("texture", &textureFileName);
    el->QueryIntAttribute("nr_shadow_steps", &nrSteps);
    
    //Create sky box mesh and read gradient texture.
    skyBoxMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
    skyBoxDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    skyBoxMesh->setDiffuseTexture(*skyBoxDiffuseTexture);
    
    skyEffect = new draw::effects::SunSkyVoxelMap(nrSteps);
    skyGradientTexture = new draw::RGBTexture2D(img::io::readImage(path + textureFileName), draw::tf::filter);
    skyEffect->setSkyTexture(*skyGradientTexture);
    skyEffect->setSun(normalize(vec3(0.4f, 0.8f, 0.4f)));
}
