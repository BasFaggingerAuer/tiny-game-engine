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

#include <tiny/img/io/image.h>

#include <tiny/draw/terrain.h>
#include <tiny/draw/heightmap/scale.h>
#include <tiny/draw/heightmap/resize.h>
#include <tiny/draw/heightmap/normalmap.h>
#include <tiny/draw/heightmap/diamondsquare.h>
#include <tiny/draw/effects/lambert.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

const vec2 terrainScale = vec2(2.0f, 2.0f);
const float terrainHeightScale = 256.0f;
draw::Terrain *terrain = 0;
draw::FloatTexture2D *terrainHeightTexture = 0;
draw::RGBTexture2D *terrainNormalTexture = 0;
draw::RGBATexture2D *terrainAttributeTexture = 0;
draw::RGBTexture2D *terrainDiffuseTexture = 0;

const ivec2 terrainFarScale = ivec2(4, 4);
const vec2 terrainFarOffset = vec2(0.5f, 0.5f);
draw::FloatTexture2D *terrainFarHeightTexture = 0;
draw::RGBTexture2D *terrainFarNormalTexture = 0;

bool terrainFollowsCamera = true;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 256.0f, 0.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create large example terrain.
    terrain = new draw::Terrain(4, 8);
    
    //Read heightmap for the far-away terrain.
    terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tasmania.png"), draw::tf::filter);
    terrainFarHeightTexture = new draw::FloatTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight(), draw::tf::filter);
    
    //Scale vertical range of the far-away heightmap.
    draw::computeScaledTexture(*terrainHeightTexture, *terrainFarHeightTexture, vec4(terrainHeightScale/255.0f), vec4(0.0f));
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*terrainFarHeightTexture, *terrainHeightTexture,
                                vec2(1.0f/static_cast<float>(terrainFarScale.x), 1.0f/static_cast<float>(terrainFarScale.y)),
                                terrainFarOffset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*terrainHeightTexture, *terrainHeightTexture, terrainFarScale.x);
    
    //Create normal maps for the far-away and zoomed-in heightmaps.
    terrainFarNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    draw::computeNormalMap(*terrainFarHeightTexture, *terrainFarNormalTexture, terrainScale.x*terrainFarScale.x);
    draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
    
    //Create simple attribute and diffuse texture.
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    terrainDiffuseTexture = new draw::RGBTexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    
    //Paint the terrain with the zoomed-in and far-away textures.
    terrain->setFarHeightTextures(*terrainHeightTexture, *terrainFarHeightTexture,
                                  *terrainNormalTexture, *terrainFarNormalTexture,
                                  terrainScale, terrainFarScale, terrainFarOffset);
    terrain->setDiffuseTexture(*terrainAttributeTexture, *terrainDiffuseTexture);
    
    //Render using Lambertian shading.
    screenEffect = new draw::effects::Lambert();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(terrain);
    worldRenderer->addScreenRenderable(screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete terrain;
    delete terrainFarHeightTexture;
    delete terrainHeightTexture;
    delete terrainFarNormalTexture;
    delete terrainNormalTexture;
    delete terrainAttributeTexture;
    delete terrainDiffuseTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Test whether we want the terrain to follow the camera.
    if (application->isKeyPressed('1'))
    {
        terrainFollowsCamera = true;
    }
    else if (application->isKeyPressed('2'))
    {
        terrainFollowsCamera = false;
    }
    
    //Update the terrain with respect to the camera.
    if (terrainFollowsCamera)
    {
        terrain->setCameraPosition(cameraPosition);
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

