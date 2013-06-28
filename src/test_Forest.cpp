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

#include <tiny/draw/computetexture.h>
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

//The Tasmania heightmap encompasses an area of about 3*10^5 metres squared in 2048x2048 pixels.
//For a zoomed-in heightmap scaling of 16, this means that we should scale the terrain by 3*10^5/(2048*16) \approx 9 for an accurate representation.
const vec2 terrainScale = vec2(3.0f, 3.0f);
//The highest mountain on Tasmania is 1617 metres, so this should be the order of magnitude for our vertical scaling.
const float terrainHeightScale = 1617.0f;
draw::Terrain *terrain = 0;
draw::FloatTexture2D *terrainHeightTexture = 0;
draw::RGBTexture2D *terrainNormalTexture = 0;

const vec2 terrainDiffuseScale = vec2(1024.0f, 1024.0f);
draw::RGBATexture2D *terrainAttributeTexture = 0;
draw::RGBTexture2D *terrainDiffuseForestTexture = 0;
draw::RGBTexture2D *terrainDiffuseGrassTexture = 0;
draw::RGBTexture2D *terrainDiffuseMudTexture = 0;
draw::RGBTexture2D *terrainDiffuseStoneTexture = 0;

const ivec2 terrainFarScale = ivec2(16, 16);
const vec2 terrainFarOffset = vec2(0.5f, 0.5f);
draw::FloatTexture2D *terrainFarHeightTexture = 0;
draw::RGBTexture2D *terrainFarNormalTexture = 0;

draw::RGBATexture2D *terrainFarAttributeTexture = 0;

bool lodFollowsCamera = true;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 256.0f, 0.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

//A GLSL program that determines the terrain type (forest/grass/mud/stone) from height information on the terrain.
template<typename TextureType1, typename TextureType2>
void computeTerrainTypeFromHeight(const TextureType1 &heightMap, TextureType2 &colourMap, const float &mapScale)
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    const std::string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceInverseSize;\n"
"uniform float mapScale;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   float height = texture(source, tex).x;\n"
"   float east = texture(source, tex + vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float west = texture(source, tex - vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float north = texture(source, tex + vec2(0.0f, sourceInverseSize.y)).x;\n"
"   float south = texture(source, tex - vec2(0.0f, sourceInverseSize.y)).x;\n"
"   \n"
"   vec3 normal = normalize(vec3(west - east, mapScale, south - north));\n"
"   \n"
"	float slope = 1.0f - normal.y;\n"
"	float forestFrac = clamp(max(0.0f, 1.0f - 9.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 450.0f)), 0.0f, 1.0f);\n"
"	float grassFrac = (1.0f - forestFrac)*clamp(max(0.0f, 1.0f - 7.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 1200.0f)), 0.0f, 1.0f);\n"
"	float mudFrac = (1.0f - grassFrac - forestFrac)*clamp(max(0.0f, 1.0f - 1.0f*slope), 0.0f, 1.0f);\n"
"	float rockFrac = 1.0f - forestFrac - grassFrac - mudFrac;\n"
"	\n"
"	colour = vec4(forestFrac, grassFrac, mudFrac, rockFrac);\n"
"}\n";
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");

    draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, outputTextures, fragmentShader);
    
    computeTexture->uniformMap().setFloatUniform(2.0f*mapScale, "mapScale");
    computeTexture->setInput(heightMap, "source");
    computeTexture->setOutput(colourMap, "colour");
    computeTexture->compute();
    
    delete computeTexture;
}

//A simple bilinear texture sampler.
template<typename TextureType>
float getHeightBilinear(const TextureType &heightMap, const vec2 &pos)
{
    //Sample heightmap at the four points surrounding pos.
    const ivec2 intPos = ivec2(floor(pos.x), floor(pos.y));
    const float h00 = heightMap(intPos.x + 0, intPos.y + 0).x;
    const float h01 = heightMap(intPos.x + 0, intPos.y + 1).x;
    const float h10 = heightMap(intPos.x + 1, intPos.y + 0).x;
    const float h11 = heightMap(intPos.x + 1, intPos.y + 1).x;
    const vec2 delta = vec2(pos.x - floor(pos.x), pos.y - floor(pos.y));
    
    //Interpolate between these four points.
    return delta.y*(delta.x*h11 + (1.0f - delta.x)*h01) + (1.0f - delta.y)*(delta.x*h10 + (1.0f - delta.x)*h00);
}

void setup()
{
    //Create large example terrain.
    terrain = new draw::Terrain(6, 8);
    
    //Read heightmap for the far-away terrain.
    terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tasmania.png"));
    terrainFarHeightTexture = new draw::FloatTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    //Scale vertical range of the far-away heightmap.
    draw::computeScaledTexture(*terrainHeightTexture, *terrainFarHeightTexture, vec4(terrainHeightScale/255.0f), vec4(0.0f));
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*terrainFarHeightTexture, *terrainHeightTexture,
                                vec2(1.0f/static_cast<float>(terrainFarScale.x), 1.0f/static_cast<float>(terrainFarScale.y)),
                                terrainFarOffset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*terrainHeightTexture, *terrainHeightTexture, terrainFarScale.x);
    //Retrieve height data for collision detection.
    terrainHeightTexture->getFromDevice();
    
    //Create normal maps for the far-away and zoomed-in heightmaps.
    terrainFarNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    draw::computeNormalMap(*terrainFarHeightTexture, *terrainFarNormalTexture, terrainScale.x*terrainFarScale.x);
    draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
    
    //Read diffuse textures and make them tileable.
    terrainDiffuseForestTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/terrain/forest.jpg"));
    terrainDiffuseGrassTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/terrain/grass.jpg"));
    terrainDiffuseMudTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/terrain/dirt.jpg"));
    terrainDiffuseStoneTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/terrain/rocks.jpg"));
    terrainDiffuseForestTexture->setAttributes(true, true, true);
    terrainDiffuseGrassTexture->setAttributes(true, true, true);
    terrainDiffuseMudTexture->setAttributes(true, true, true);
    terrainDiffuseStoneTexture->setAttributes(true, true, true);
    
    //Create an attribute texture that determines the terrain type (forest/grass/mud/stone) based on the altitude and slope.
    //We do this for both the zoomed-in and far-away terrain.
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth(), 255, 0, 0, 0));
    terrainFarAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    
    computeTerrainTypeFromHeight(*terrainHeightTexture, *terrainAttributeTexture, terrainScale.x);
    computeTerrainTypeFromHeight(*terrainFarHeightTexture, *terrainFarAttributeTexture, terrainScale.x*terrainFarScale.x);
    
    //Paint the terrain with the zoomed-in and far-away textures.
    terrain->setFarHeightTextures(*terrainHeightTexture, *terrainFarHeightTexture,
                                  *terrainNormalTexture, *terrainFarNormalTexture,
                                  terrainScale, terrainFarScale, terrainFarOffset);
    terrain->setFarDiffuseTextures(*terrainAttributeTexture, *terrainFarAttributeTexture,
                                   *terrainDiffuseForestTexture, *terrainDiffuseGrassTexture, *terrainDiffuseMudTexture, *terrainDiffuseStoneTexture,
                                   terrainDiffuseScale);
    
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
    
    delete terrainFarAttributeTexture;
    delete terrainAttributeTexture;
    delete terrainDiffuseForestTexture;
    delete terrainDiffuseGrassTexture;
    delete terrainDiffuseMudTexture;
    delete terrainDiffuseStoneTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //If the camera is below the terrain, increase its height.
    const vec2 cameraPositionOnTerrain((cameraPosition.x/terrainScale.x) + 0.5f*static_cast<float>(terrainHeightTexture->getWidth()),
                                       (cameraPosition.z/terrainScale.y) + 0.5f*static_cast<float>(terrainHeightTexture->getHeight()));
    const float terrainHeight = getHeightBilinear(*terrainHeightTexture, cameraPositionOnTerrain) + 2.0f;
    
    cameraPosition.y = std::max(cameraPosition.y, terrainHeight);
    
    //Test whether we want the terrain to follow the camera.
    if (application->isKeyPressed('1'))
    {
        lodFollowsCamera = true;
    }
    else if (application->isKeyPressed('2'))
    {
        lodFollowsCamera = false;
    }
    
    //Update the terrain with respect to the camera.
    if (lodFollowsCamera)
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

