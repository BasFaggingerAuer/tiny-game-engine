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
#include <tiny/mesh/animatedmesh.h>
#include <tiny/mesh/io/animatedmesh.h>

#include <tiny/draw/animatedmeshhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::AnimatedMeshInstance> testMeshInstances;
draw::AnimatedMeshHorde *testMeshHorde = 0;
draw::AnimationTextureBuffer *testAnimations = 0;
draw::RGBTexture2D *testDiffuseTexture = 0;
draw::RGBTexture2D *testNormalTexture = 0;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 10.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a test mesh and paint it with a texture.
    mesh::AnimatedMesh animatedMesh = mesh::io::readAnimatedMesh(DATA_DIRECTORY + "mesh/cubes.dae");
    const size_t nrBones = animatedMesh.skeleton.bones.size();
    const size_t nrFrames = animatedMesh.skeleton.animations.begin()->second.frames.size();
    
    //Create a test mesh and paint it with a texture.
    testMeshHorde = new draw::AnimatedMeshHorde(animatedMesh, 1024);
    testAnimations = new draw::AnimationTextureBuffer();
    testAnimations->setAnimations(animatedMesh.skeleton.animations.begin(), animatedMesh.skeleton.animations.end());
    testDiffuseTexture = new draw::RGBTexture2D(img::Image::createTestImage(64));
    testNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    testMeshHorde->setAnimationTexture(*testAnimations);
    testMeshHorde->setDiffuseTexture(*testDiffuseTexture);
    testMeshHorde->setNormalTexture(*testNormalTexture);
    
    //Create instances of the tests in a grid.
    const float meshSpacing = 2.0f;
    
    for (int i = -4; i <= 4; ++i)
    {
        for (int j = -4; j <= 4; ++j)
        {
            for (int k = -4; k <= 4; ++k)
            {
                testMeshInstances.push_back(draw::AnimatedMeshInstance(vec4(meshSpacing*i, meshSpacing*j, meshSpacing*k, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), ivec2(3*nrBones*(rand() % nrFrames), 0)));
            }
        }
    }
    
    testMeshHorde->setMeshes(testMeshInstances.begin(), testMeshInstances.end());
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the test and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(testMeshHorde);
    worldRenderer->addScreenRenderable(screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete testMeshHorde;
    delete testAnimations;
    delete testDiffuseTexture;
    delete testNormalTexture;
}

void update(const double &dt)
{
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

