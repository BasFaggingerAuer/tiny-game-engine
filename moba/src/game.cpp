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
#include <tiny/mesh/io/animatedmesh.h>

#include <tiny/snd/worldsounderer.h>

#include "game.h"

using namespace moba;
using namespace tiny;

Game::Game(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight()))
{
    readResources(path);
    
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    unsigned int index = 0;
    
    renderer->addWorldRenderable(index++, skyBoxMesh);
    
    renderer->addWorldRenderable(index++, terrain->terrain);
    
    renderer->addWorldRenderable(index++, forest->treeMeshes, true, true, draw::BlendReplace, draw::CullNothing);
    renderer->addWorldRenderable(index++, forest->treeSprites);
    
    for (std::vector<Faction *>::const_iterator i = factions.begin(); i != factions.end(); ++i)
    {
        renderer->addWorldRenderable(index++, (*i)->towerMeshes);
    }
    
    for (std::map<std::string, MinionType *>::const_iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        renderer->addWorldRenderable(index++, i->second->horde);
    }
    
    renderer->addScreenRenderable(index++, skyEffect, false, false);
    
    clear();
    
    //Plant some trees.
    forest->plantTrees(terrain);
    
    //Create a minion.
    moba.insert(std::pair<unsigned int, Minion>(0, Minion("Dummy", "Wolf")));
    
    //Create faction structures.
    for (std::vector<Faction *>::iterator i = factions.begin(); i != factions.end(); ++i)
    {
        (*i)->plantBuildings(terrain);
    }
}

Game::~Game()
{
    clear();
    
    delete renderer;
    
    delete skyBoxMesh;
    delete skyBoxDiffuseTexture;
    delete skyGradientTexture;
    delete skyEffect;
    
    for (std::map<std::string, MinionType *>::const_iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        delete i->second;
    }
    
    delete terrain;
    delete forest;
}

void Game::update(os::Application *application, const float &dt)
{
    //Update moba.
    
    //Clear all instance lists.
    for (std::map<std::string, MinionType *>::iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        i->second->instances.clear();
    }
    
    //Update moba and fill instance lists.
    for (std::map<unsigned int, Minion>::iterator i = moba.begin(); i != moba.end(); ++i)
    {
        Minion m = i->second;
        assert(minionTypes.find(m.type) != minionTypes.end());
        MinionType *mt = minionTypes[m.type];
        draw::AnimatedMeshInstance mi;
        
        //TODO: Separate this into a separate update-minion stage and an update-horde stage.
        //TODO: Store FPS in animation somehow.
        const float fps = 10.0f;
        
        m.actionTime += dt;
        
        assert(mt->mesh.skeleton.animations.find(m.action) != mt->mesh.skeleton.animations.end());
        
        const int nrAnimationFrames = mt->mesh.skeleton.animations[m.action].frames.size()/mt->mesh.skeleton.bones.size();
        const int frame = static_cast<int>(floor(m.actionTime*fps)) % nrAnimationFrames;
        
        mt->instances.push_back(draw::AnimatedMeshInstance(vec4(m.pos.x, terrain->getHeight(m.pos), m.pos.y, 1.0f), quatrot(m.angle, vec3(0.0f, 1.0f, 0.0f)), ivec2(3*mt->mesh.skeleton.bones.size()*frame, 0)));
        
        i->second = m;
    }
    
    //Send instances to the GPU.
    for (std::map<std::string, MinionType *>::iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    //Move the camera around and collide with the terrain.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    cameraPosition.y = std::max(cameraPosition.y, terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f);
    
    //Update the terrain with respect to the camera.
    terrain->terrain->setCameraPosition(cameraPosition);
    
    forest->setCameraPosition(cameraPosition);
    
    //Tell the world renderer that the camera has changed.
    renderer->setCamera(cameraPosition, cameraOrientation);
    snd::WorldSounderer::setCamera(cameraPosition, cameraOrientation);
}

void Game::render()
{
    renderer->clearTargets();
    renderer->render();
}

void Game::clear()
{
    //Stops any running game and clears all data.

    //Reset camera.
    cameraPosition = vec3(0.0f, 0.0f, 0.0f);
    cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    moba.clear();
}

void Game::readResources(const std::string &path)
{
    const std::string worldFileName = path + "moba.xml";
    
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
    
    if (root->ValueStr() != "moba")
    {
        std::cerr << "This is not a valid moba XML file!" << std::endl;
        throw std::exception();
    }
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
        if (el->ValueStr() == "sky")
        {
            readSkyResources(path, el);
        }
        else if (el->ValueStr() == "terrain")
        {
            terrain = new GameTerrain(path, el);
        }
        else if (el->ValueStr() == "forest")
        {
            forest = new GameForest(path, el);
        }
        else if (el->ValueStr() == "minion_type")
        {
            MinionType *minionType = new MinionType(path, el);
            
            if (minionTypes.find(minionType->name) != minionTypes.end())
            {
                std::cerr << "Minion type '" << minionType->name << "' already exists!" << std::endl;
                throw std::exception();
            }
            
            minionTypes.insert(std::pair<std::string, MinionType *>(minionType->name, minionType));
        }
        else if (el->ValueStr() == "faction")
        {
            factions.push_back(new Faction(path, el));
        }
    }
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

