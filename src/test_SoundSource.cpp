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

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/smp/sample.h>
#include <tiny/snd/source.h>
#include <tiny/snd/buffer.h>
#include <tiny/snd/worldsounderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

draw::StaticMesh *cubeMesh = 0;
draw::RGBATexture2D *cubeDiffuseTexture = 0;

draw::Renderable *screenEffect = 0;

snd::Source *cubeSoundSource = 0;
snd::MonoSoundBuffer *cubeSoundBuffer = 0;
const size_t soundFrequency = 44100; //Hz

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Play sound from the cube.
    cubeSoundSource = new snd::Source(vec3(0.0f, 0.0f, 0.0f));
    cubeSoundBuffer = new snd::MonoSoundBuffer(smp::Sample::createTone());
    cubeSoundSource->playBuffer(*cubeSoundBuffer, true);
    
    //Create a cube mesh and paint it with a texture.
    cubeMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(0.5f));
    cubeDiffuseTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    cubeMesh->setDiffuseTexture(*cubeDiffuseTexture);
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(0, cubeMesh);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete cubeSoundSource;
    delete cubeSoundBuffer;
    
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
    snd::WorldSounderer::setCamera(cameraPosition, cameraOrientation);
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

