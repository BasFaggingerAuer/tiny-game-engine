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

CharacterType::CharacterType(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading character type resource..." << std::endl;
   
    assert(std::string(el->Value()) == "charactertype");
    
    name = "";
    size = vec3(1.0f);
    float scaleHeight = 0.125f;
    
    nrInstances = 0;
    maxNrInstances = 1;
    
    el->QueryStringAttribute("name", &name);
    el->QueryFloatAttribute("width", &size.x);
    el->QueryFloatAttribute("height", &size.y);
    el->QueryFloatAttribute("depth", &size.z);
    el->QueryFloatAttribute("scale_height", &scaleHeight);
    
    std::string diffuseFileName = "";
    std::string normalFileName = "";
    std::string meshFileName = "";
    
    el->QueryStringAttribute("diffuse", &diffuseFileName);
    el->QueryStringAttribute("normal", &normalFileName);
    el->QueryStringAttribute("mesh", &meshFileName);
    el->QueryIntAttribute("max_instances", &maxNrInstances);
    
    //Read mesh and textures.
    diffuseTexture = new draw::RGBTexture2D(diffuseFileName.empty() ? img::Image::createSolidImage() : img::io::readImage(path + diffuseFileName));
    normalTexture = new draw::RGBTexture2D(normalFileName.empty() ? img::Image::createUpNormalImage() : img::io::readImage(path + normalFileName));
    mesh::StaticMesh mesh = (meshFileName.empty() ? mesh::StaticMesh::createBoxMesh(1.0f, 1.0f, 1.0f) : mesh::io::readStaticMesh(path + meshFileName));
    
    //Center mesh and fit it to size.
    const std::pair<vec3, vec3> boundingBox = mesh.getBoundingBox();
    
    //Extract bottom 12.5% of the mesh to use for centering and scaling.
    mesh::StaticMesh lowerMesh;
    const float lowerY = boundingBox.first.y + (boundingBox.second.y - boundingBox.first.y)*scaleHeight;
    
    for (auto i = mesh.vertices.cbegin(); i != mesh.vertices.cend(); ++i)
    {
        if (i->position.y <= lowerY)
        {
            lowerMesh.vertices.push_back(*i);
        }
    }
    
    const std::pair<vec3, vec3> lowerBoundingBox = lowerMesh.getBoundingBox();
    const vec3 o = vec3(0.5f*(lowerBoundingBox.first.x + lowerBoundingBox.second.x), lowerBoundingBox.first.y, 0.5f*(lowerBoundingBox.first.z + lowerBoundingBox.second.z));
    const float s = minComponent(abs(size.xz() - 0.1f)/abs(lowerBoundingBox.second.xz() - lowerBoundingBox.first.xz()));
    
    for (std::vector<mesh::StaticMeshVertex>::iterator i = mesh.vertices.begin(); i != mesh.vertices.end(); ++i)
    {
        i->position = s*(i->position - o);
    }
    
    horde = new draw::StaticMeshHorde(mesh, maxNrInstances);
    horde->setDiffuseTexture(*diffuseTexture);
    horde->setNormalTexture(*normalTexture);
    instances.resize(maxNrInstances);
    
    shadowDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage());
    shadowNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    
    shadowHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createBoxMesh(0.5f*size.x - 0.05f, 0.05f, 0.5f*size.z - 0.05f), maxNrInstances);
    shadowHorde->setDiffuseTexture(*shadowDiffuseTexture);
    shadowHorde->setNormalTexture(*shadowNormalTexture);
    shadowInstances.resize(maxNrInstances);
    
    std::cerr << "Added '" << name << "' character type." << std::endl;
}

CharacterType::~CharacterType()
{
    delete horde;
    delete diffuseTexture;
    delete normalTexture;
    
    delete shadowHorde;
    delete shadowDiffuseTexture;
    delete shadowNormalTexture;
}

void CharacterType::clearInstances()
{
    nrInstances = 0;
}

void CharacterType::addInstance(const CharacterInstance &instance, const float &baseHeight)
{
    if (nrInstances < maxNrInstances)
    {
        instances[nrInstances] = draw::StaticMeshInstance(vec4(instance.position.x, instance.position.y + baseHeight, instance.position.z, 1.0f),
                                                            quatrot((M_PI/180.0f)*instance.rotation, vec3(0.0f, 1.0f, 0.0f)),
                                                            instance.getColor());
        shadowInstances[nrInstances] = draw::StaticMeshInstance(vec4(instance.position.x, baseHeight, instance.position.z, 1.0f),
                                                            vec4(0.0f, 0.0f, 0.0f, 1.0f),
                                                            instance.getColor());
        ++nrInstances;
    }
}

void CharacterType::updateInstances()
{
    horde->setMeshes(instances.begin(), instances.begin() + nrInstances);
    shadowHorde->setMeshes(shadowInstances.begin(), shadowInstances.begin() + nrInstances);
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
    
    renderer->addWorldRenderable(index++, voxelMap);
    
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
    delete voxelTexture;
    delete voxelCubeArrayTexture;
    
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
            
            strm << "\\#fff" << i->first << (i->first == characterIndex ? "*(" : " (") << std::dec << std::setprecision(0) << std::fixed << 5.0f*i->second.position.x << ", " << 5.0f*i->second.position.y << ", " << 5.0f*i->second.position.z << "): \\#" << std::hex << std::min(15, static_cast<int>(16.0f*c.x)) << std::min(15, static_cast<int>(16.0f*c.y)) << std::min(15, static_cast<int>(16.0f*c.z)) << i->second.name << std::endl;
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
    
    for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
    {
        assert(characterTypes.find(i->second.type) != characterTypes.end());
        characterTypes[i->second.type]->addInstance(i->second, 0.0f);
    }
    
    for (std::map<unsigned int, CharacterType *>::iterator i = characterTypes.begin(); i != characterTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    //Draw character icons.
    std::vector<draw::WorldIconInstance> iconInstances;
    const float fontHeightScale = 0.15f/fontTexture->getMaxIconDimensions().y;
    
    for (std::map<unsigned int, CharacterInstance>::const_iterator i = characters.begin(); i != characters.end(); ++i)
    {
        if (!i->second.name.empty())
        {
            const vec4 icon = fontTexture->getIcon(i->second.name[0]);
            
            iconInstances.push_back(draw::WorldIconInstance(
                vec3(i->second.position.x, i->second.position.y + 0.5f*characterTypes[i->second.type]->size.y, i->second.position.z),
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
        //Release control of characters.
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
            //Did the user click anywhere?
            auto mouse = application->getMouseState(false);
            
            if (mouse.buttons != 0)
            {
                if (mouseReleased)
                {
                    mouseReleased = false;
                    
                    //Translate from screen to world coordinates.
                    auto voxelHit = voxelMap->getIntersection(*voxelTexture, cameraPosition, renderer->getWorldDirection(vec2(mouse.x, 1.0f - mouse.y)));
                    
                    if (voxelHit.distance > 0.0f)
                    {
                        if (mouse.buttons == 1)
                        {
                            ivec3 p = max(ivec3(0), min(ivec3(voxelTexture->getWidth() - 1, voxelTexture->getHeight() - 1, voxelTexture->getDepth() - 1),
                                          voxelHit.voxelIndices + voxelHit.normal));
                            (*voxelTexture)[2*(p.z*voxelTexture->getHeight()*voxelTexture->getWidth() + p.y*voxelTexture->getWidth() + p.x) + 0] = 1;
                        }
                        else if (mouse.buttons == 2)
                        {
                            ivec3 p = max(ivec3(0), min(ivec3(voxelTexture->getWidth() - 1, voxelTexture->getHeight() - 1, voxelTexture->getDepth() - 1),
                                          voxelHit.voxelIndices));
                            (*voxelTexture)[2*(p.z*voxelTexture->getHeight()*voxelTexture->getWidth() + p.y*voxelTexture->getWidth() + p.x) + 0] = 0;
                        }
                        
                        voxelTexture->sendToDevice();
                    }
                }
            }
            else
            {
                mouseReleased = true;
            }
            
            //We do not control a character: control the camera directly.
            //Move the camera around and collide with the terrain.
            const float cameraRadius = 1.0f;
            vec3 newPosition = cameraPosition;
            
            application->updateSimpleCamera(dt, newPosition, cameraOrientation);
            newPosition.y = std::max(newPosition.y, terrain->getHeight(vec2(newPosition.x, newPosition.z)) + cameraRadius);
            newPosition.x = clamp(newPosition.x, -0.5f*voxelMap->getScale()*static_cast<float>(voxelTexture->getWidth()), 0.5f*voxelMap->getScale()*static_cast<float>(voxelTexture->getWidth()));
            newPosition.y = clamp(newPosition.y, 0.0f, voxelMap->getScale()*static_cast<float>(voxelTexture->getHeight()));
            newPosition.z = clamp(newPosition.z, -0.5f*voxelMap->getScale()*static_cast<float>(voxelTexture->getDepth()), 0.5f*voxelMap->getScale()*static_cast<float>(voxelTexture->getDepth()));
            cameraPosition = newPosition;
        }
        else
        {
            //We do control a character.
            CharacterInstance &chr = characters[characterIndex];
            
            if (application->isKeyPressedOnce('s')) chr.position.x = roundf(chr.position.x + 1.0f);
            if (application->isKeyPressedOnce('w')) chr.position.x = roundf(chr.position.x - 1.0f);
            if (application->isKeyPressedOnce('a')) chr.position.z = roundf(chr.position.z + 1.0f);
            if (application->isKeyPressedOnce('d')) chr.position.z = roundf(chr.position.z - 1.0f);
            if (application->isKeyPressedOnce('q')) chr.position.y = roundf(chr.position.y + 1.0f);
            if (application->isKeyPressedOnce('e')) chr.position.y = std::max(0.0f, roundf(chr.position.y - 1.0f));
            if (application->isKeyPressedOnce('j')) chr.rotation += 90.0f;
            if (application->isKeyPressedOnce('l')) chr.rotation -= 90.0f;
            
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
        else if (std::string(el->Value()) == "voxelmap") readVoxelMapResources(path, el);
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
    skyEffect->setVoxelMap(*voxelTexture, voxelMap->getScale());
}

void Game::readVoxelMapResources(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading voxel map resources..." << std::endl;
    
    assert(std::string(el->Value()) == "voxelmap");
    
    int width = 64;
    int height = 64;
    int depth = 64;
    float voxelSize = 1.0f;
    
    el->QueryIntAttribute("width", &width);
    el->QueryIntAttribute("height", &height);
    el->QueryIntAttribute("depth", &depth);
    el->QueryFloatAttribute("scale", &voxelSize);
    
    voxelMap = new draw::VoxelMap(std::max(width, std::max(height, depth))*4);
    voxelTexture = new draw::RGTexture3D(width, height, depth, draw::tf::none);
    
    for (size_t z = 0; z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 0; y < voxelTexture->getHeight(); ++y)
        {
            for (size_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                (*voxelTexture)[2*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = (y > 0 ? 0 : 1);
                (*voxelTexture)[2*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 1] = (((x ^ y ^ z) & 1) == 0 ? 128 : 160);
            }
        }
    }
    
    voxelTexture->sendToDevice();
    
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    
    //Read cubemaps.
    std::vector<img::Image> cubeMaps;
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (std::string(sl->Value()) == "cubemap")
        {
            std::string textureFileName = "";
            
            sl->QueryStringAttribute("all", &textureFileName);
            
            if (!textureFileName.empty())
            {
                auto texture = img::io::readImage(path + textureFileName);
                
                for (int im = 0; im < 6; ++im)
                {
                    cubeMaps.push_back(texture);
                }
            }
            else
            {
                sl->QueryStringAttribute("px", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mx", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("py", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("my", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("pz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
            }
        }
        else
        {
            std::cerr << "Warning: unknown data " << sl->Value() << " encountered in XML!" << std::endl;
        }
    }
    
    voxelCubeArrayTexture = new draw::RGBTexture2DCubeArray(cubeMaps.begin(), cubeMaps.end(), draw::tf::repeat | draw::tf::mipmap);
    
    voxelMap->setCubeMaps(*voxelCubeArrayTexture);
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
    
    baseCharacters.push_back(CharacterInstance(typeIndex, name, vec3(x, y, z), r, c));
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

