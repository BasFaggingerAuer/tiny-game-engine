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

CollisionHashMap::CollisionHashMap(const size_t &a_nrBuckets, const size_t &a_p1, const size_t &a_p2, const float &a_size) :
    nrBuckets(a_nrBuckets),
    p1(a_p1),
    p2(a_p2),
    size(a_size),
    cylinders(),
    buckets()
{

}

CollisionHashMap::~CollisionHashMap()
{

}

void CollisionHashMap::buildCollisionBuckets(const std::vector<tiny::vec4> &collisionCylinders)
{
    //Allocate collision detection buckets.
    buckets = std::vector<std::list<int> >(nrBuckets, std::list<int>());
    
    //Assign all cylinders to their buckets.
    cylinders = collisionCylinders;
    
    for (std::vector<tiny::vec4>::const_iterator i = collisionCylinders.begin(); i != collisionCylinders.end(); ++i)
    {
        //A single cylinder can cover multiple buckets.
        const int minX = static_cast<int>(floor((i->x - i->w)/size));
        const int maxX = static_cast<int>(floor((i->x + i->w)/size));
        const int minY = static_cast<int>(floor((i->z - i->w)/size));
        const int maxY = static_cast<int>(floor((i->z + i->w)/size));
        
        for (int j = minX; j <= maxX; ++j)
        {
            for (int k = minY; k <= maxY; ++k)
            {
                buckets[(p1*static_cast<size_t>(j) + p2*static_cast<size_t>(k)) & (nrBuckets - 1)].push_back(i - collisionCylinders.begin());
            }
        }
    }
    
    /*
    size_t nrEmpty = 0;
    
    for (std::vector<std::list<int> >::const_iterator i = buckets.begin(); i != buckets.end(); ++i)
    {
        if (i->empty())
        {
            ++nrEmpty;
        }
    }
    
    std::cerr << nrEmpty << "/" << buckets.size() << " empty buckets, " << cylinders.size()/buckets.size() << " cylinders per bucket." << std::endl;
    */
}

vec2 CollisionHashMap::projectVelocity(const vec2 &pos, const float &radius, vec2 vel) const
{
    //Project velocity away from cylinders with which we collide.
    //A single cylinder can cover multiple buckets.
    const float r = length(vel) + radius;
    const int minX = static_cast<int>(floor((pos.x - r)/size));
    const int maxX = static_cast<int>(floor((pos.x + r)/size));
    const int minY = static_cast<int>(floor((pos.y - r)/size));
    const int maxY = static_cast<int>(floor((pos.y + r)/size));
    
    for (int j = minX; j <= maxX; ++j)
    {
        for (int k = minY; k <= maxY; ++k)
        {
            const std::list<int> &l = buckets[(p1*static_cast<size_t>(j) + p2*static_cast<size_t>(k)) & (nrBuckets - 1)];
            
            for (std::list<int>::const_iterator m = l.begin(); m != l.end(); ++m)
            {
                vel = projectVelocityCylinder(cylinders[*m], pos, radius, vel);
            }
        }
    }
    
    return vel;
}

vec2 CollisionHashMap::projectVelocityCylinder(const vec4 &cyl, const vec2 &pos, const float &radius, vec2 vel) const
{
    vec2 delta = vec2(cyl.x, cyl.z) - pos;
    const float r1 = length(delta);
    const float r2 = cyl.w + radius;
    
    if (r1 <= r2 && r1 >= 1.0e-6)
    {
        //We are inside the cylinder, so we can only move away from its center.
        delta = delta/r1;
        vel = vel - std::max(dot(delta, vel), 0.0f)*delta;
    }
    
    return vel;
}

Game::Game(const os::Application *application, const std::string &path) :
    aspectRatio(static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight())),
    collisionHandler(1024, 3, 7, 16.0f)
{
    menuCameraPosition = vec3(0.0f, 0.0f, 0.0f);
    menuCameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    spawnCameraPosition = vec3(0.0f, 0.0f, 0.0f);
    spawnCameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    spawnTime = 1.0f;
    
    readResources(path);
    
    renderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    unsigned int index = 0;
    
    renderer->addWorldRenderable(index++, skyBoxMesh);
    
    renderer->addWorldRenderable(index++, terrain->terrain);
    
    renderer->addWorldRenderable(index++, forest->treeMeshes, true, true, draw::BlendMode::BlendReplace, draw::CullMode::CullNothing);
    renderer->addWorldRenderable(index++, forest->treeSprites);
    
    for (std::map<std::string, Faction *>::const_iterator i = factions.begin(); i != factions.end(); ++i)
    {
        renderer->addWorldRenderable(index++, i->second->nexusMesh);
        renderer->addWorldRenderable(index++, i->second->towerMeshes);
    }
    
    for (std::map<std::string, MinionType *>::const_iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        renderer->addWorldRenderable(index++, i->second->horde);
    }
    
    renderer->addScreenRenderable(index++, skyEffect, false, false);
    renderer->addScreenRenderable(index++, logoLayer, false, false, draw::BlendMode::BlendMix);
    
    clear();
    
    //Plant some trees.
    if (true)
    {
        std::list<vec4> collisionEntities = forest->plantTrees(terrain);
        
        staticCollisionCylinders.splice(staticCollisionCylinders.end(), collisionEntities);
    }
    
    //Convert minion paths.
    for (std::map<std::string, MinionPath *>::iterator i = minionPaths.begin(); i != minionPaths.end(); ++i)
    {
        i->second->plantNodes(terrain);
    }
    
    //Create a minion.
    spawnMinionAtPath("Wolfie", "Wolf", "RadiantMid");
    
    //Create faction structures.
    for (std::map<std::string, Faction *>::iterator i = factions.begin(); i != factions.end(); ++i)
    {
        std::list<vec4> collisionEntities = i->second->plantBuildings(terrain);
        
        staticCollisionCylinders.splice(staticCollisionCylinders.end(), collisionEntities);
    }
}

void Game::spawnMinionAtPath(const std::string &name, const std::string &minionType, const std::string &path, const float &radius)
{
    //Spawn a minion at the start of a certain path.
    
    //Check that we are given valid data.
    assert(minionTypes.find(minionType) != minionTypes.end());
    assert(minionPaths.find(path) != minionPaths.end());
    
    Minion minion = Minion(name, minionType, minionPaths[path]->nodes[0] + randomVec2(radius));
    
    minion.path = path;
    minions.insert(std::make_pair(minionIndex++, minion));

    std::cerr << "Spawned minion " << name << " of type " << minionType << " at path " << path << "." << std::endl;
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
    
    for (std::map<std::string, MinionPath *>::const_iterator i = minionPaths.begin(); i != minionPaths.end(); ++i)
    {
        delete i->second;
    }
    
    for (std::map<std::string, Faction *>::const_iterator i = factions.begin(); i != factions.end(); ++i)
    {
        delete i->second;
    }
    
    delete terrain;
    delete forest;
    
    delete logoLayer;
    delete logoTexture;
    delete giftTexture;
}

std::vector<vec4> Game::createCollisionCylinders() const
{
    //Create a list of all objects that can be collided with.
    std::vector<vec4> cylinders;
    
    cylinders.reserve(staticCollisionCylinders.size() + minions.size());
    cylinders.insert(cylinders.begin(), staticCollisionCylinders.begin(), staticCollisionCylinders.end());
    
    for (std::map<unsigned int, Minion>::const_iterator i = minions.begin(); i != minions.end(); ++i)
    {
        const Minion m = i->second;
        std::map<std::string, MinionType *>::const_iterator j = minionTypes.find(m.type);
        assert(j != minionTypes.end());
        const MinionType *mt = j->second;
        
        cylinders.push_back(vec4(m.pos.x, terrain->getHeight(m.pos), m.pos.y, mt->radius));
    }
    
    return cylinders;
}

void Game::update(os::Application *application, const float &dt)
{
    //Update minions.
    
    //Clear all instance lists.
    for (std::map<std::string, MinionType *>::iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        i->second->instances.clear();
    }
    
    //Do we need to spawn minions?
    for (std::map<std::string, Faction *>::iterator i = factions.begin(); i != factions.end(); ++i)
    {
        for (std::list<MinionSpawner>::iterator j = i->second->minionSpawners.begin(); j != i->second->minionSpawners.end(); ++j)
        {
            j->currentTime -= dt;
            
            //Is it time to spawn a new wave?
            if (j->currentTime <= 0.0f)
            {
                j->currentTime = j->cooldownTime;
                
                for (int k = 0; k < j->nrSpawn; ++k)
                {
                    spawnMinionAtPath("Unnamed", j->minionType, j->pathName, j->radius);
                }
            }
        }
    }
    
    //Update collision detection.
    if (true)
    {
        std::vector<vec4> cylinders = createCollisionCylinders();
        
        cylinders.push_back(vec4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 0.5f));
        
        collisionHandler.buildCollisionBuckets(cylinders);
    }
    
    //Update minions and fill instance lists.
    for (std::map<unsigned int, Minion>::iterator i = minions.begin(); i != minions.end(); )
    {
        Minion m = i->second;
        assert(minionTypes.find(m.type) != minionTypes.end());
        MinionType *mt = minionTypes[m.type];
        draw::AnimatedMeshInstance mi;
        
        //Increment action time.
        bool isErased = false;
        
        m.actionTime += dt;
        
        //Follow the assigned path if any.
        if (m.path != "")
        {
            const MinionPath *p = minionPaths[m.path];
            
            //Have we reached the current node?
            if (length(m.pos - p->nodes[m.pathIndex]) < 16.0f)
            {
                m.pathIndex++;
            }
            
            if (m.pathIndex >= p->nodes.size())
            {
                //We have reached the end of the path --> remove the minion.
                isErased = true;
                std::cerr << "Removed minion " << m.name << "." << std::endl;
            }
            else
            {
                //Head for the next node.
                const vec2 d = normalize(p->nodes[m.pathIndex] - m.pos);
                vec2 vel = mt->maxSpeed*dt*d;
                
                vel = collisionHandler.projectVelocity(m.pos, mt->radius, vel);
                m.pos += vel;
                m.angle = atan2f(d.y, d.x) - M_PI/2.0;
            }
        }
        
        if (isErased)
        {
            minions.erase(i++);
        }
        else
        {
            i->second = m;
            ++i;
        }
        
        //TODO: Separate this into a separate update-minion stage and an update-horde stage.
        //TODO: Store FPS in animation somehow.
        const float fps = 20.0f;
        
        assert(mt->mesh.skeleton.animations.find(m.action) != mt->mesh.skeleton.animations.end());
        
        const int nrAnimationFrames = mt->mesh.skeleton.animations[m.action].frames.size()/mt->mesh.skeleton.bones.size();
        const int frame = static_cast<int>(floor(m.actionTime*fps)) % nrAnimationFrames;
        
        mt->instances.push_back(draw::AnimatedMeshInstance(vec4(m.pos.x, terrain->getHeight(m.pos), m.pos.y, 1.0f), quatrot(m.angle, vec3(0.0f, 1.0f, 0.0f)), ivec2(3*mt->mesh.skeleton.bones.size()*frame, 0)));
    }
    
    //Send instances to the GPU.
    for (std::map<std::string, MinionType *>::iterator i = minionTypes.begin(); i != minionTypes.end(); ++i)
    {
        i->second->updateInstances();
    }
    
    if (gameMode == 0)
    {
        //Menu mode.
        if (application->isKeyPressed(' '))
        {
            gameMode = 1;
        }
    }
    else if (gameMode == 1)
    {
        //Spawn mode.
        if (currentSpawnTime < spawnTime)
        {
            const float s = currentSpawnTime/spawnTime;
            
            logoLayer->setAlpha(1.0f - s);
            
            cameraPosition = (1.0f - s)*menuCameraPosition + s*spawnCameraPosition;
            cameraOrientation = normalize((1.0f - s)*menuCameraOrientation + s*spawnCameraOrientation);
            
            currentSpawnTime += dt;
        }
        else
        {
            gameMode = 2;
            logoLayer->setImageTexture(*giftTexture);
        }
    }
    else
    {
        //Run around mode.
        
        //Update the position and orientation of a simple controllable camera.
        const float ds = (application->isKeyPressed('f') ? 32.0f : 8.0f)*dt;
        const float dr = 2.1f*dt;
        
        if (application->isKeyPressed('i')) cameraOrientation = quatmul(quatrot(dr, vec3(-1.0f, 0.0f, 0.0f)), cameraOrientation);
        if (application->isKeyPressed('k')) cameraOrientation = quatmul(quatrot(dr, vec3( 1.0f, 0.0f, 0.0f)), cameraOrientation);
        if (application->isKeyPressed('j')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f,-1.0f, 0.0f)), cameraOrientation);
        if (application->isKeyPressed('l')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 1.0f, 0.0f)), cameraOrientation);
        if (application->isKeyPressed('u')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f,-1.0f)), cameraOrientation);
        if (application->isKeyPressed('o')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f, 1.0f)), cameraOrientation);
    
        cameraOrientation = normalize(cameraOrientation);
    
        vec3 vel = mat4(cameraOrientation)*vec3((application->isKeyPressed('d') && application->isKeyPressed('a')) ? 0.0f : (application->isKeyPressed('d') ? 1.0f : (application->isKeyPressed('a') ? -1.0f : 0.0f)),
                                                (application->isKeyPressed('q') && application->isKeyPressed('e')) ? 0.0f : (application->isKeyPressed('q') ? 1.0f : (application->isKeyPressed('e') ? -1.0f : 0.0f)),
                                                (application->isKeyPressed('s') && application->isKeyPressed('w')) ? 0.0f : (application->isKeyPressed('s') ? 1.0f : (application->isKeyPressed('w') ? -1.0f : 0.0f)));
        
        vel = ds*normalize(vel);
        
        //Perform collision detection.
        const vec2 vel2 = collisionHandler.projectVelocity(vec2(cameraPosition.x, cameraPosition.z), 0.5f, vec2(vel.x, vel.z));
        
        cameraPosition += vec3(vel2.x, vel.y, vel2.y);
        //cameraPosition.y = std::max(cameraPosition.y, terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f);
        cameraPosition.y = terrain->getHeight(vec2(cameraPosition.x, cameraPosition.z)) + 2.0f;
        
        //Did we reach the other nexus?
        if (true)
        {
            float d = length(cameraPosition - vec3(496.0f, 45.5f, -504.0f));
            
            d = std::min(std::max(0.0f, 1.1f - (d/30.0f)), 1.0f);
            logoLayer->setAlpha(d);
        }
    }
    
    /*
    if (application->isKeyPressed(' '))
    {
        std::cout << "position_x=\"" << cameraPosition.x << "\" position_y=\"" << cameraPosition.y << "\" position_z=\"" << cameraPosition.z << "\" orientation_x=\"" << cameraOrientation.x << "\" orientation_y=\"" << cameraOrientation.y << "\" orientation_z=\"" << cameraOrientation.z << "\" orientation_w=\"" << cameraOrientation.w << "\"" << std::endl;
    }
    */
    
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
    gameMode = 0;
    cameraPosition = menuCameraPosition;
    cameraOrientation = menuCameraOrientation;
    currentSpawnTime = 0.0f;
    
    minions.clear();
    minionIndex = 0;
}

void Game::readResources(const std::string &path)
{
    img::Image logoImage = img::Image::createSolidImage();
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
    
    //Read logo.
    if (root->Attribute("logo")) logoImage = img::io::readImage(path + std::string(root->Attribute("logo")));
    
    logoTexture = new tiny::draw::RGBATexture2D(logoImage, tiny::draw::tf::filter);
    giftTexture = new tiny::draw::RGBATexture2D(img::io::readImage(path + "gift.png"), tiny::draw::tf::filter);
    logoLayer = new tiny::draw::effects::ShowImage();
    logoLayer->setImageTexture(*logoTexture);
    logoLayer->setAspectRatio(aspectRatio);
    
    //Read all parts of the XML file.
    for (TiXmlElement *el = root->FirstChildElement(); el; el = el->NextSiblingElement())
    {
        if (el->ValueStr() == "menu_camera")
        {
            el->QueryFloatAttribute("position_x", &menuCameraPosition.x);
            el->QueryFloatAttribute("position_y", &menuCameraPosition.y);
            el->QueryFloatAttribute("position_z", &menuCameraPosition.z);
            el->QueryFloatAttribute("orientation_x", &menuCameraOrientation.x);
            el->QueryFloatAttribute("orientation_y", &menuCameraOrientation.y);
            el->QueryFloatAttribute("orientation_z", &menuCameraOrientation.z);
            el->QueryFloatAttribute("orientation_w", &menuCameraOrientation.w);
            menuCameraOrientation = normalize(menuCameraOrientation);
        }
        else if (el->ValueStr() == "spawn_camera")
        {
            el->QueryFloatAttribute("position_x", &spawnCameraPosition.x);
            el->QueryFloatAttribute("position_y", &spawnCameraPosition.y);
            el->QueryFloatAttribute("position_z", &spawnCameraPosition.z);
            el->QueryFloatAttribute("orientation_x", &spawnCameraOrientation.x);
            el->QueryFloatAttribute("orientation_y", &spawnCameraOrientation.y);
            el->QueryFloatAttribute("orientation_z", &spawnCameraOrientation.z);
            el->QueryFloatAttribute("orientation_w", &spawnCameraOrientation.w);
            spawnCameraOrientation = normalize(spawnCameraOrientation);
            el->QueryFloatAttribute("spawn_time", &spawnTime);
        }
        else if (el->ValueStr() == "sky")
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
        else if (el->ValueStr() == "minion_path")
        {
            MinionPath *minionPath = new MinionPath(path, el);
            
            if (minionPaths.find(minionPath->name) != minionPaths.end())
            {
                std::cerr << "Minion path '" << minionPath->name << "' already exists!" << std::endl;
                throw std::exception();
            }
            
            minionPaths.insert(std::pair<std::string, MinionPath *>(minionPath->name, minionPath));
        }
        else if (el->ValueStr() == "faction")
        {
            Faction *faction = new Faction(path, el);
            
            if (factions.find(faction->name) != factions.end())
            {
                std::cerr << "Minion path '" << faction->name << "' already exists!" << std::endl;
                throw std::exception();
            }
            
            factions.insert(std::pair<std::string, Faction *>(faction->name, faction));
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

