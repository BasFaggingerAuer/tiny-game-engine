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
    tankIndex(0)
{

}

Player::~Player()
{

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
    
    clear();
}

TanksGame::~TanksGame()
{
    clear();
    
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

void TanksGame::update(os::Application *application, const float &dt)
{
    //Exchange network data.
    if (host) host->listen(0.0);
    if (client) client->listen(0.0);

    //Update all tanks.
    for (std::map<unsigned int, TankInstance>::iterator i = tanks.begin(); i != tanks.end(); ++i)
    {
        TankInstance &t = i->second;
        const TankType *tt = tankTypes[t.type];
        vec3 F(0.0f); //force
        vec3 tau(0.0f); //torque
        
        //Thrusters.
        if (t.controls != 0)
        {
            const mat4 ori(t.q);
            
            for (int j = 0; j < 4; ++j)
            {
                if (t.controls & (1 << j))
                {
                    F += ori*tt->thrust_force[j];
                    tau += cross(ori*tt->thrust_pos[j], ori*tt->thrust_force[j]);
                }
            }
        }
        else
        {
            //Spinning dampeners.
        }
        
        //Gravity acts depending on the height above the terrain.
        float height = t.x.y - terrain->getHeight(vec2(t.x.x, t.x.z));
        float gravityFactor = 1.0f;
        
        if (height <= tt->radius1)
        {
            //Tank rests on solid core, normal force counters gravity.
            gravityFactor = 0.0f;
        }
        else if (height <= tt->radius2)
        {
            //Dampening field counters gravity via a spring-like force.
            gravityFactor = 1.0f - (height - tt->radius1)/(tt->radius2 - tt->radius1);
        }
        
        F += vec3(0.0f, -9.81f*gravityFactor*tt->mass, 0.0f);
        
        //The normal force countering gravity causes drag (average metal-wood static friction coefficient is 0.4).
        //TODO: Calculate normal to the terrain and do proper dry friction. Some terrain types are slippery?
        F -= (1.0f - gravityFactor)*0.4f*t.P;
        
        //Air friction causes drag.
        //F_drag = 0.5*air density*velocity^2*drag coefficient*cross section area.
        //air density ~ 1.3 kg/m^3
        //drag coefficient ~ 0.5
        //cross section = pi*r^2.
        const vec3 v = t.P/tt->mass;
        
        F -= (0.5f*1.3f*length2(v)*0.5f*3.1415f*tt->radius1*tt->radius1)*normalize(v);
        
        //Air friction also causes rotational drag.
        //tau_drag = -8*pi*r^3*air viscosity*angular velocity.
        //air viscosity ~ 1.5e-5 kg/(m*s).
        const vec3 omega = t.L/tt->inertia;
        
        tau -= (8.0f*3.1415f*tt->radius1*tt->radius1*tt->radius1*static_cast<float>(1.5e-5))*omega;
        
        //Integrate forces via the semi-implicit Euler integrator (which is symplectic).
        t.P += dt*F;
        t.L += dt*tau;
        t.x += (dt/tt->mass)*t.P;
        t.q += (0.5f*dt/tt->inertia)*quatmul(vec4(t.L.x, t.L.y, t.L.z), t.q);
        t.q = normalize(t.q);
        
        //Tanks may never go beneath the terrain.
        height = terrain->getHeight(vec2(t.x.x, t.x.z)) + tt->radius1;
        
        if (height > t.x.y)
        {
            t.x.y = height;
            //Bounce off the terrain if we are crashing into it too fast.
            if (t.P.y < 0.0f) t.P.y *= -0.5f;
        }
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
        //Do we control a tank?
        unsigned int tankIndex = 0;
        
        if (players.find(ownPlayerIndex) != players.end())
        {
            tankIndex = players[ownPlayerIndex].tankIndex;
            
            if (tanks.find(tankIndex) == tanks.end())
            {
                //If we have received a nonzero tank id, it should always be valid!
                assert(tankIndex == 0);
                tankIndex = 0;
            }
        }
        
        if (tankIndex == 0)
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
            
            if (application->isKeyPressed('a')) controls |=   1;
            if (application->isKeyPressed('d')) controls |=   2;
            if (application->isKeyPressed('w')) controls |=   4;
            if (application->isKeyPressed('s')) controls |=   8;
            
            TankInstance &tank = tanks[tankIndex];
            
            if (controls != tank.controls)
            {
                tank.controls = controls;
                
                //Is this a network game?
                if (host || client)
                {
                    Message msg(msg::mt::updateTank);
                    
                    msg << tankIndex << tank.controls << tank.x << tank.q << tank.P << tank.L;
                    
                    if (client) client->sendMessage(msg);
                    if (host) host->sendMessage(msg);
                }
            }
            
            //Look from our tank.
            cameraPosition = tank.x;
            cameraOrientation = tank.q;
        }
        
        //Update the terrain with respect to the camera.
        terrain->terrain->setCameraPosition(cameraPosition);
        
        //Tell the world renderer that the camera has changed.
        renderer->setCamera(cameraPosition, cameraOrientation);
    }
}

void TanksGame::render()
{
    renderer->clearTargets();
    renderer->render();
}

void TanksGame::clear()
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
    
    //Remove all tanks.
    tanks.clear();
    lastTankIndex = 1;
    
    //Reset camera.
    cameraPosition = vec3(0.0f, 0.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}

TanksMessageTranslator *TanksGame::getTranslator() const
{
    return translator;
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

