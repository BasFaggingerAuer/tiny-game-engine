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
#include <string>
#include <exception>

#include <config.h>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include <tiny/img/image.h>
#include <tiny/mesh/staticmesh.h>

#include <tiny/lod/quadtree.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

#define NR_DETAIL_LEVELS 4

const float lodRadius = 16.0f;
const int maxNrCubesPerLOD = 1024;
draw::StaticMeshHorde *cubeMeshHorde[NR_DETAIL_LEVELS] = {0};
draw::RGBATexture2D *cubeDiffuseTexture[NR_DETAIL_LEVELS] = {0};
std::vector<draw::StaticMeshInstance> allCubeMeshInstances;
std::vector<draw::StaticMeshInstance> lodCubeMeshInstances;
std::vector<int> lodCubeMeshIndices;

lod::Quadtree *quadtree = 0;

bool lodFollowsCamera = true;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a cube mesh and paint it with a different texture for each detail level, with a random colour.
    srand(1234567890);
    
    for (int i = 0; i < NR_DETAIL_LEVELS; ++i)
    {
        cubeMeshHorde[i] = new draw::StaticMeshHorde(mesh::StaticMesh::createCubeMesh(0.5f), maxNrCubesPerLOD);
        cubeDiffuseTexture[i] = new draw::RGBATexture2D(img::Image::createSolidImage(16,
                                                                                     ((i & 1) == 0 ? 255 : 0), ((i & 2) == 0 ? 255 : 0) , ((i & 4) == 0 ? 255 : 0)));
        cubeMeshHorde[i]->setDiffuseTexture(*cubeDiffuseTexture[i]);
    }
    
    //Temporary arrays in which we accumulate all data from the cubes in a specific level of detail.
    lodCubeMeshInstances.resize(maxNrCubesPerLOD);
    lodCubeMeshIndices.resize(maxNrCubesPerLOD);
    
    //Create instances of the cubes in a grid.
    std::vector<vec3> allCubeMeshPositions;
    const int gridSize = 24;
    const float meshSpacing = 0.5f*lodRadius;
    
    for (int i = -gridSize; i <= gridSize; ++i)
    {
        for (int j = -gridSize; j <= gridSize; ++j)
        {
            const vec3 position = vec3(meshSpacing*i, 0.0f, meshSpacing*j);
            
            allCubeMeshInstances.push_back(draw::StaticMeshInstance(vec4(position.x, position.y, position.z, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f)));
            allCubeMeshPositions.push_back(position);
        }
    }
    
    //Build quadtree of all cube positions.
    quadtree = new lod::Quadtree();
    
    quadtree->buildQuadtree(allCubeMeshPositions.begin(), allCubeMeshPositions.end());
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube detail levels and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    
    for (int i = 0; i < NR_DETAIL_LEVELS; ++i)
    {
        worldRenderer->addWorldRenderable(i, cubeMeshHorde[i]);
    }
    
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete quadtree;
    
    for (int i = 0; i < NR_DETAIL_LEVELS; ++i)
    {
        delete cubeMeshHorde[i];
        delete cubeDiffuseTexture[i];
    }
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Test whether we want the level of detail management to follow the camera.
    if (application->isKeyPressed('1'))
    {
        lodFollowsCamera = true;
    }
    else if (application->isKeyPressed('2'))
    {
        lodFollowsCamera = false;
    }
    
    if (lodFollowsCamera)
    {
        //Populate detail levels with cubes depending on the distance from the camera.
        float minRadius = 0.0f;
        float deltaRadius = lodRadius;
        
        //Verify that we assign each object to a single detail level.
        std::vector<bool> wasAssigned(allCubeMeshInstances.size(), false);
        int nrDoublyAssigned = 0;
        
        for (int i = 0; i < NR_DETAIL_LEVELS; ++i)
        {
            //Retrieve indices of all cubes between the minimum and maximum radius for this level of detail.
            const int nrLODInstances = quadtree->retrieveIndicesBetweenRadii(cameraPosition,
                                                                             minRadius, minRadius + deltaRadius,
                                                                             lodCubeMeshIndices.begin(), maxNrCubesPerLOD)
                                       - lodCubeMeshIndices.begin();
            
            assert(nrLODInstances <= maxNrCubesPerLOD);
            
            //Gather cube instances corresponding to these indices.
            for (int j = 0; j < nrLODInstances; ++j)
            {
                assert(lodCubeMeshIndices[j] >= 0 && lodCubeMeshIndices[j] < static_cast<int>(allCubeMeshInstances.size()));
                lodCubeMeshInstances[j] = allCubeMeshInstances[lodCubeMeshIndices[j]];
                
                if (wasAssigned[lodCubeMeshIndices[j]])
                {
                    ++nrDoublyAssigned;
                }
                
                wasAssigned[lodCubeMeshIndices[j]] = true;
            }
            
            //Send these to the appropriate level of detail.
            cubeMeshHorde[i]->setMeshes(lodCubeMeshInstances.begin(), lodCubeMeshInstances.begin() + nrLODInstances);
            
            minRadius += deltaRadius;
            deltaRadius *= 1.5f;
        }
        
        if (nrDoublyAssigned > 0)
        {
            std::cerr << "Warning: " << nrDoublyAssigned << " objects assigned to multiple LOD levels!" << std::endl;
        }
    }
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
}

void render()
{
    worldRenderer->clearTargets();
    worldRenderer->render();
}

int main(int, char **)
{
    try
    {
        application = new os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        setup();
    }
    catch (std::exception &e)
    {
        cerr << "Unable to start application!" << endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        update(application->pollEvents());
        render();
        application->paint();
    }
    
    cleanup();
    delete application;
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

