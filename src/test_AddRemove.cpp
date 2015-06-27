/*
Copyright 2012-2015, Bas Fagginger Auer and Matthijs van Dorp.

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

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

draw::StaticMesh *cubeMesh = 0;
draw::RGBATexture2D *cubeDiffuseTexture = 0;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

float currentTime = 0.0f;
float flickerTime = 0.01f;
bool meshExists = false;

void createMesh()
{
    //Create a cube mesh and paint it with a texture.
    cubeMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(0.5f));
    cubeMesh->setDiffuseTexture(*cubeDiffuseTexture);
    
    worldRenderer->addWorldRenderable(0, cubeMesh);
    meshExists = true;
}

void deleteMesh()
{
    worldRenderer->freeWorldRenderable(0);
    delete cubeMesh; cubeMesh = 0;
    meshExists = false;
}

void setup()
{
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addScreenRenderable(1, screenEffect, false, false);

    cubeDiffuseTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    createMesh();
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete cubeMesh;
    delete cubeDiffuseTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);

    currentTime += dt;
    if( (int(currentTime/flickerTime) % 2) == 0 && !meshExists) createMesh();
    else if( (int(currentTime/flickerTime) % 2) == 1 && meshExists)
    {
        deleteMesh();
//        for(unsigned int i = 0; i < 20; i++) { createMesh(); deleteMesh(); }
    }
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

