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
#include <tiny/img/io/image.h>
#include <tiny/mesh/animatedmesh.h>
#include <tiny/mesh/io/animatedmesh.h>

#include <tiny/draw/animatedmesh.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

draw::AnimatedMesh *testMesh = 0;
draw::AnimationTextureBuffer *testAnimations = 0;
draw::RGBTexture2D *testDiffuseTexture = 0;
draw::RGBTexture2D *testNormalTexture = 0;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

double globalTime = 0.0;
int testNrFrames = 1;

void setup(const std::string &fileName)
{
    //Create a test mesh and paint it with a texture.
    mesh::AnimatedMesh animatedMesh = mesh::io::readAnimatedMesh(fileName);
    
    //Create simple dummy animation if none exist.
    if (animatedMesh.skeleton.animations.empty())
    {
        const size_t nrBones = animatedMesh.skeleton.bones.size();
        
        testNrFrames = 32;
        mesh::Animation animation;
        
        animation.name = "Dummy";
        animation.frames.assign(testNrFrames*nrBones, mesh::KeyFrame());
        
        for (int i = 0; i < testNrFrames; ++i)
        {
            for (size_t j = 0; j < nrBones; ++j)
            {
                animation.frames[nrBones*i + j] = mesh::KeyFrame(vec3(0.1f + static_cast<float>(i)/static_cast<float>(testNrFrames)), i, quatrot(6.3f*static_cast<float>(i)/static_cast<float>(testNrFrames), vec3(0.0f, 1.0f, 0.0f)), vec3(0.0f, 0.0f, 0.1f*sin(i)));
            }
        }
        
        animatedMesh.skeleton.animations.push_back(animation);
    }
    else
    {
        testNrFrames = std::max<int>(1, animatedMesh.skeleton.animations[0].frames.size()/animatedMesh.skeleton.bones.size());
    }
    
    testMesh = new draw::AnimatedMesh(animatedMesh);
    testAnimations = new draw::AnimationTextureBuffer();
    testAnimations->setAnimations(animatedMesh.skeleton.animations.begin(), animatedMesh.skeleton.animations.end());
    testDiffuseTexture = new draw::RGBTexture2D(img::Image::createTestImage(64));
    testNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    testMesh->setAnimationTexture(*testAnimations);
    testMesh->setDiffuseTexture(*testDiffuseTexture);
    testMesh->setNormalTexture(*testNormalTexture);
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the test and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(testMesh);
    worldRenderer->addScreenRenderable(screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete testMesh;
    delete testAnimations;
    delete testDiffuseTexture;
    delete testNormalTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Let the user select a frame of the animation.
    globalTime += dt;
    
    const int frame = static_cast<int>(floor(4.0*globalTime)) % testNrFrames;
    
    testMesh->setAnimationFrame(frame);
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
}

void render()
{
    worldRenderer->clearTargets();
    worldRenderer->render();
}

int main(int argc, char **argv)
{
    try
    {
        application = new os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        setup(argc > 1 ? argv[1] : DATA_DIRECTORY + "mesh/cubes.dae");
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

