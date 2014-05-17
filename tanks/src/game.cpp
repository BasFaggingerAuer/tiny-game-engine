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
    soldierIndex(0)
{

}

Player::~Player()
{

}

Game::Game(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight())),
    mouseSensitivity(48.0),
    gravitationalConstant(9.81),
    lastSoldierIndex(1),
    lastBulletIndex(1),
    lastExplosionIndex(1),
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
    
    renderer->addWorldRenderable(skyBoxMesh);
    renderer->addWorldRenderable(terrain->terrain);
    
    for (std::map<unsigned int, SoldierType *>::const_iterator i = soldierTypes.begin(); i != soldierTypes.end(); ++i)
    {
        renderer->addWorldRenderable(i->second->horde);
    }
    
    
    renderer->addScreenRenderable(skyEffect, false, false);
    renderer->addScreenRenderable(bulletHorde, true, false, tiny::draw::BlendAdd);
    renderer->addScreenRenderable(consoleBackground, false, false, draw::BlendMix);
    renderer->addScreenRenderable(font, false, false, draw::BlendMix);
    
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
    
    for (std::map<unsigned int, SoldierType *>::iterator i = soldierTypes.begin(); i != soldierTypes.end(); ++i)
    {
        delete i->second;
    }
    
    for (std::map<unsigned int, BulletType *>::iterator i = bulletTypes.begin(); i != bulletTypes.end(); ++i)
    {
        delete i->second;
    }
    
    for (std::map<unsigned int, ExplosionType *>::iterator i = explosionTypes.begin(); i != explosionTypes.end(); ++i)
    {
        delete i->second;
    }
    
    delete bulletHorde;
    delete bulletIconTexture;
}

void Game::updateConsole() const
{
    //Update the console on screen.
    font->setText(-1.0, -1.0, consoleMode ? 0.05 : 0.025, aspectRatio, console->getText(256), *fontTexture);
    consoleBackground->setColour(consoleMode ? vec4(0.0f, 0.0f, 0.0f, 0.8f) : vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

void Game::update(os::Application *application, const float &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Update soldiers.
    for (std::map<unsigned int, SoldierInstance>::iterator i = soldiers.begin(); i != soldiers.end(); ++i)
    {
        SoldierInstance &t = i->second;
        const SoldierType *tt = soldierTypes[t.type];
        
        //Get orientation.
        //TODO: Express this in terms of t.angles?
        t.q = normalize(t.q);
        
        const mat4 ori(t.q);
        bool airborne = false;
        
        if (t.x.y <= terrain->getHeight(vec2(t.x.x, t.x.z)) + 0.01f)
        {
            const float l = length(t.P);
            
            if (l > 0.0f)
            {
                //Decellerate via dry friction.
                const vec3 frictionForce = -(dt*tt->mass*gravitationalConstant*tt->landFriction/l)*t.P;
                
                if (length(frictionForce) >= length(t.P))
                {
                    //We come to a standstill.
                    t.P = vec3(0.0f);
                }
                else
                {
                    //Apply force.
                    t.P += frictionForce;
                }
            }
            
            //Are we jumping?
            if (t.controls & 15)
            {
                //We are walking.
                vec2 move = vec2(0.0f, 0.0f);
                const vec2 forward = vec2(ori.v02, ori.v22);
                const vec2 right = vec2(ori.v00, ori.v20);
                
                if (t.controls & 1) move -= forward;
                if (t.controls & 2) move += forward;
                if (t.controls & 4) move -= right;
                if (t.controls & 8) move += right;
                
                move = dt*tt->mass*tt->speed*normalize(move);
                t.P.x += move.x;
                t.P.z += move.y;
            }
            
            if ((t.controls & 16) && t.P.y < 0.01f)
            {
                t.P.y = tt->mass*tt->jump;
                airborne = true;
            }
        }
        else
        {
            //Decellerate via air friction.
            t.P -= (dt*tt->airFriction*length(t.P))*t.P;
            
            //Gravity pulls the player down.
            t.P.y -= dt*tt->mass*gravitationalConstant;
            airborne = true;
        }
        
        //Integrate position.
        t.x += (dt/tt->mass)*t.P;
        
        //The player should never go below the terrain.
        if (airborne)
        {
            t.x.y = std::max(t.x.y, terrain->getHeight(vec2(t.x.x, t.x.z)));
        }
        else
        {
            t.x.y = terrain->getHeight(vec2(t.x.x, t.x.z));
        }
        
        //Decrease weapon recharge times.
        assert(t.weaponRechargeTimes.size() == tt->weapons.size());
        
        for (std::vector<float>::iterator j = t.weaponRechargeTimes.begin(); j != t.weaponRechargeTimes.end(); ++j)
        {
            if (*j > 0.0f) *j -= dt;
        }
    }
    
    //Let explosions and soldiers interact.
    //TODO: Do this with a more clever collision scheme.
    for (std::map<unsigned int, SoldierInstance>::iterator i = soldiers.begin(); i != soldiers.end(); ++i)
    {
        SoldierInstance &t = i->second;
        const SoldierType *tt = soldierTypes[t.type];
        
        for (std::map<unsigned int, ExplosionInstance>::iterator j = explosions.begin(); j != explosions.end(); ++j)
        {
            ExplosionInstance &e = j->second;
            assert(explosionTypes.find(e.type) != explosionTypes.end());
            const ExplosionType *et = explosionTypes[e.type];
            vec3 delta = t.x - e.x;
            
            if (length(delta) <= e.r + tt->radius)
            {
                delta = normalize(delta);
                t.P += dt*et->push*delta;
            }
        }
    }

    //Draw soldiers.
    for (std::map<unsigned int, SoldierType *>::iterator i = soldierTypes.begin(); i != soldierTypes.end(); ++i)
    {
        i->second->clearInstances();
    }
    
    for (std::map<unsigned int, SoldierInstance>::const_iterator i = soldiers.begin(); i != soldiers.end(); ++i)
    {
        assert(soldierTypes.find(i->second.type) != soldierTypes.end());
        soldierTypes[i->second.type]->addInstance(i->second);
    }
    
    for (std::map<unsigned int, SoldierType *>::iterator i = soldierTypes.begin(); i != soldierTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    //Update bullets.
    for (std::map<unsigned int, BulletInstance>::iterator i = bullets.begin(); i != bullets.end(); ++i)
    {
        BulletInstance &t = i->second;
        
        t.lifetime -= dt;
        t.v += dt*t.a;
        t.x += dt*t.v;
    }
    
    //Draw bullets.
    unsigned int nrBulletInstances = 0;
    
    for (std::map<unsigned int, BulletInstance>::iterator i = bullets.begin(); i != bullets.end() && nrBulletInstances < bulletInstances.size(); )
    {
        assert(bulletTypes.find(i->second.type) != bulletTypes.end());
        const BulletType *tt = bulletTypes[i->second.type];
        bool destroyed = false;
        
        //Destroy the bullet if it is below the terrain.
        if (i->second.x.y < terrain->getHeight(vec2(i->second.x.x, i->second.x.z)) + tt->radius || i->second.lifetime <= 0.0f)
        {
            destroyed = true;
        }
        
        if (!destroyed)
        {
            bulletInstances[nrBulletInstances++] = tiny::draw::WorldIconInstance(i->second.x, tiny::vec2(2.0f*tt->radius, 2.0f*tt->radius), tt->icon, tiny::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            ++i;
        }
        else
        {
            //Create an explosion.
            Message msg(msg::mt::addExplosion);
            
            msg << lastExplosionIndex++ << i->second.explosionType << i->second.x;
            userMessage(msg);
            
            //Destroy the bullet instance.
            bullets.erase(i++);
        }
    }
    
    //Update explosions.
    for (std::map<unsigned int, ExplosionInstance>::iterator i = explosions.begin(); i != explosions.end(); ++i)
    {
        ExplosionInstance &t = i->second;
        assert(explosionTypes.find(t.type) != explosionTypes.end());
        const ExplosionType *tt = explosionTypes[t.type];
        
        t.r += dt*tt->expansionSpeed;
    }
    
    //Draw explosions.
    for (std::map<unsigned int, ExplosionInstance>::iterator i = explosions.begin(); i != explosions.end() && nrBulletInstances < bulletInstances.size(); )
    {
        assert(explosionTypes.find(i->second.type) != explosionTypes.end());
        const ExplosionType *tt = explosionTypes[i->second.type];
        bool destroyed = false;
        
        //Destroy the explosion if it has expanded too much.
        if (i->second.r >= tt->maxRadius)
        {
            destroyed = true;
        }
        
        if (!destroyed)
        {
            const float a = (i->second.r - tt->minRadius)/(tt->maxRadius - tt->minRadius);
            
            bulletInstances[nrBulletInstances++] = tiny::draw::WorldIconInstance(i->second.x, tiny::vec2(2.0f*i->second.r, 2.0f*i->second.r), tt->icon, tiny::vec4(1.0f - a, 1.0f - a, 1.0f - a, 1.0f));
            ++i;
        }
        else
        {
            explosions.erase(i++);
        }
    }
    
    bulletHorde->setIcons(bulletInstances.begin(), bulletInstances.begin() + nrBulletInstances);
    
    //Toggle console.
    if (application->isKeyPressedOnce('`'))
    {
        consoleMode = !consoleMode;
        updateConsole();
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
        //Do we control a soldier?
        unsigned int soldierIndex = 0;
        
        if (players.find(ownPlayerIndex) != players.end())
        {
            soldierIndex = players[ownPlayerIndex].soldierIndex;
            
            if (soldiers.find(soldierIndex) == soldiers.end())
            {
                //If we have received a nonzero soldier id, it should always be valid!
                assert(soldierIndex == 0);
                soldierIndex = 0;
            }
        }
        
        if (soldierIndex == 0)
        {
            //We do not control a soldier: control the camera directly.
            //Move the camera around and collide with the terrain.
            application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
            cameraPosition.y = std::max(cameraPosition.y, terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f);
        }
        else
        {
            //Update the controls of our soldier and send these to the host if we are a client.
            const tiny::os::MouseState mouseState = application->getMouseState(true);
            const vec2 mouseDelta = vec2(mouseState.x, mouseState.y);
            unsigned int controls = 0;
            
            if (application->isKeyPressed('w')) controls |=  1;
            if (application->isKeyPressed('s')) controls |=  2;
            if (application->isKeyPressed('a')) controls |=  4;
            if (application->isKeyPressed('d')) controls |=  8;
            if (application->isKeyPressed(' ')) controls |= 16;
            
            SoldierInstance &soldier = soldiers[soldierIndex];
            const SoldierType *soldierType = soldierTypes[soldier.type];
            
            //Update orientation.
            soldier.angles += mouseSensitivity*dt*mouseDelta;
            soldier.angles.y = clamp(soldier.angles.y, -1.2f, 1.2f);
            soldier.q = quatrot(soldier.angles.x, vec3(0.0f, 1.0f, 0.0f));
            
            if (controls != soldier.controls || length2(mouseDelta) > 0.0f)
            {
                soldier.controls = controls;
                
                //Is this a network game?
                if (host || client)
                {
                    Message msg(msg::mt::updateSoldier);
                    
                    msg << soldierIndex << soldier.controls << soldier.angles << soldier.x << soldier.q << soldier.P;
                    
                    if (client) client->sendMessage(msg);
                    if (host) host->sendMessage(msg);
                }
            }
            
            //Do we want to shoot?
            if (mouseState.buttons != 0 && !soldier.weaponRechargeTimes.empty())
            {
                //Is our weapon charged?
                const unsigned int w = 0;
                
                if (soldier.weaponRechargeTimes[w] <= 0.0f)
                {
                    Message msg(msg::mt::playerShootRequest);
                    
                    msg << w;
                    
                    userMessage(msg);
                    soldier.weaponRechargeTimes[w] = soldierType->weapons[w].rechargeTime;
                }
            }
            
            //Look from our soldier.
            cameraPosition = soldierType->getCameraPosition(soldier);
            cameraOrientation = soldierType->getCameraOrientation(soldier);
        }
        
        //Update the terrain with respect to the camera.
        terrain->terrain->setCameraPosition(cameraPosition);
        terrain->updateSoundVolume(vec2(cameraPosition.x, cameraPosition.z));
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
    }
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
    
    //Remove all soldiers.
    soldiers.clear();
    lastSoldierIndex = 1;
    
    //Remove all bullets.
    bullets.clear();
    lastBulletIndex = 1;
    
    //Remove all explosions.
    explosions.clear();
    lastExplosionIndex = 1;
    
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
        std::cerr << "This is not a valid tanks XML file!" << std::endl;
        throw std::exception();
    }
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
             if (el->ValueStr() == "console") readConsoleResources(path, el);
        else if (el->ValueStr() == "sky") readSkyResources(path, el);
        else if (el->ValueStr() == "terrain") terrain = new GameTerrain(path, el);
        else if (el->ValueStr() == "soldier") soldierTypes[soldierTypes.size()] = new SoldierType(path, el);
        else if (el->ValueStr() == "bullethorde") readBulletHordeResources(path, el);
        else if (el->ValueStr() == "bullet") bulletTypes[bulletTypes.size()] = new BulletType(path, el);
        else if (el->ValueStr() == "explosion") explosionTypes[explosionTypes.size()] = new ExplosionType(path, el);
    }
    
    //Pack all bullet and explosion images into a single large texture.
    for (std::map<unsigned int, BulletType *>::iterator i = bulletTypes.begin(); i != bulletTypes.end(); ++i)
    {
        i->second->icon = bulletIconTexture->packIcon(*(i->second->bulletImage));
    }
    
    for (std::map<unsigned int, ExplosionType *>::iterator i = explosionTypes.begin(); i != explosionTypes.end(); ++i)
    {
        i->second->icon = bulletIconTexture->packIcon(*(i->second->explodeImage));
    }
    
    //Match soldier weapons to read bullet types.
    for (std::map<unsigned int, SoldierType *>::iterator i = soldierTypes.begin(); i != soldierTypes.end(); ++i)
    {
        for (std::vector<SoldierWeapon>::iterator j = i->second->weapons.begin(); j != i->second->weapons.end(); ++j)
        {
            bool foundBullet = false;
            bool foundExplosion = false;
            unsigned int bulletIndex = 0;
            unsigned int explosionIndex = 0;
            
            for (std::map<unsigned int, BulletType *>::iterator k = bulletTypes.begin(); k != bulletTypes.end() && !foundBullet; ++k)
            {
                if (k->second->name == j->bulletName)
                {
                    foundBullet = true;
                    bulletIndex = k->first;
                }
            }
            
            for (std::map<unsigned int, ExplosionType *>::iterator k = explosionTypes.begin(); k != explosionTypes.end() && !foundExplosion; ++k)
            {
                if (k->second->name == j->explosionName)
                {
                    foundExplosion = true;
                    explosionIndex = k->first;
                }
            }
            
            if (!foundBullet || !foundExplosion)
            {
                std::cerr << "Unable to find bullet type '" << j->bulletName << "' or explosion '" << j->explosionName << "' for weapon '" << j->name << "' of soldier type '" << i->second->name << "'!" << std::endl;
                
                //Erase weapon.
                j = i->second->weapons.erase(j);
            }
            else
            {
                j->bulletType = bulletIndex;
                j->explosionType = explosionIndex;
            }
        }
    }
}

void Game::readConsoleResources(const std::string &path, TiXmlElement *el)
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
    font->setText(-1.0, -1.0, 0.1, aspectRatio, "Welcome to \\rTanks\\w, press ` to access the console.", *fontTexture);
    consoleBackground = new draw::effects::Solid();
}

void Game::readSkyResources(const std::string &path, TiXmlElement *el)
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

void Game::readBulletHordeResources(const std::string &, TiXmlElement *el)
{
    std::cerr << "Reading bullet horde resources..." << std::endl;
    
    std::string textureFileName = "";
    
    assert(el->ValueStr() == "bullethorde");
    
    int bulletTextureSize = 0;
    int maxNrBulletInstances = 0;
    
    el->QueryIntAttribute("texture_size", &bulletTextureSize);
    el->QueryIntAttribute("max_instances", &maxNrBulletInstances);
    
    //Create bullet icon texture and horde.
    bulletIconTexture = new tiny::draw::IconTexture2D(bulletTextureSize, bulletTextureSize);
    bulletHorde = new tiny::draw::WorldIconHorde(maxNrBulletInstances, true);
    bulletHorde->setIconTexture(*bulletIconTexture);
    bulletInstances.resize(maxNrBulletInstances);
}

