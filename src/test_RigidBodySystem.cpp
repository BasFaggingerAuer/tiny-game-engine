/*
Copyright 2018, Bas Fagginger Auer.

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

#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/rigid/rigidbody.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::StaticMeshInstance> sphereMeshInstances;
draw::StaticMeshHorde *sphereMeshHorde = 0;
draw::RGBATexture2D *sphereDiffuseTexture = 0;

rigid::RigidBodySystem *rigidBodySystem = 0;
float lastEnergyTime = -10.0f;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 10.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a rigid body scene.
    const int nrBalls = 4;
    
    rigidBodySystem = new rigid::RigidBodySystem();
    rigidBodySystem->addRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 0.25f)}, vec3(-1.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
    
    for (int i = 0; i < nrBalls; ++i)
    {
        rigidBodySystem->addRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 0.25f)}, vec3(0.55f*static_cast<float>(i), 0.0f, 0.0f));
    }
    
    rigidBodySystem->addRigidBody(1.0e8f, {vec4(0.0f, 0.0f, 0.0f, 0.25f)}, vec3(0.55f*static_cast<float>(nrBalls + 1), 0.0f, 0.0f));

    rigidBodySystem->addRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 0.5f),
                                         vec4(1.0f, 0.0f, 0.0f, 0.5f),
                                         vec4(2.0f, 0.0f, 0.0f, 0.5f),
                                         vec4(3.0f, 0.0f, 0.0f, 0.5f),
                                         vec4(0.0f, -1.0f, 0.0f, 0.5f),
                                         vec4(0.0f, -2.0f, 0.0f, 0.5f),
                                         vec4(0.0f, -3.0f, 0.0f, 0.5f),
                                         },
                                         vec3(-5.0f, 1.5f, 0.0f), vec3(0.0f, 0.0f, 0.0f));
//                                         vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.1f, 1.0f, 0.0f));
    
    //Create a cube mesh and paint it with a texture.
    sphereMeshHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createIcosahedronMesh(1.0f), 1024);
    sphereDiffuseTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    sphereMeshHorde->setDiffuseTexture(*sphereDiffuseTexture);
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(0, sphereMeshHorde);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete sphereMeshHorde;
    delete sphereDiffuseTexture;
    
    delete rigidBodySystem;
}

void update(const double &dt)
{
    //Update the rigid bodies.
    rigidBodySystem->update(dt);

    if (rigidBodySystem->getTime() > lastEnergyTime + 0.5f)
    {
        lastEnergyTime = rigidBodySystem->getTime();
        std::cout << "Total energy: " << rigidBodySystem->getTotalEnergy() << "." << std::endl;
    }
    
    //Get rigid body positions and send them to the static mesh horde.
    sphereMeshInstances.clear();
    rigidBodySystem->getInternalSphereStaticMeshes(sphereMeshInstances);
    sphereMeshHorde->setMeshes(sphereMeshInstances.begin(), sphereMeshInstances.end());
    
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
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

