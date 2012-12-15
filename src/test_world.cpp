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
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/draw/worldrenderer.h>
#include <tiny/draw/worldeffectrenderer.h>
#include <tiny/draw/screensquare.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/lighthorde.h>
#include <tiny/draw/terrain.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

double aspectRatio = 1.0;
draw::WorldRenderer *worldRenderer = 0;
draw::WorldEffectRenderer *effectRenderer = 0;

draw::StaticMesh *testMesh = 0;
draw::RGBATexture2D *testDiffuseTexture = 0;

draw::Terrain *terrain = 0;

std::vector<draw::PointLightInstance> pointLightInstances;
draw::PointLightHorde *pointLights = 0;

draw::RGBATexture2D *diffuseTexture = 0;
draw::Vec4Texture2D *worldNormalTexture = 0;
draw::Vec4Texture2D *worldPositionTexture = 0;
draw::DepthTexture2D *depthTexture = 0;

draw::ScreenIconHorde *font = 0;
draw::IconTexture2D *fontTexture = 0;
draw::WorldIconHorde *worldFont = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 0.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

double globalTime = 0.0;

class SimpleFogEffect : public tiny::draw::ScreenFillingSquare
{
    public:
        SimpleFogEffect();
        ~SimpleFogEffect();
        
        std::string getFragmentShaderCode() const;
        
        void setSun(const vec3 &);
        void setFog(const float &);
};

SimpleFogEffect::SimpleFogEffect() :
    ScreenFillingSquare()
{
    setSun(vec3(0.7f, 0.7f, 0.0f));
    setFog(256.0f);
}

SimpleFogEffect::~SimpleFogEffect()
{

}

std::string SimpleFogEffect::getFragmentShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D worldNormalTexture;\n"
"uniform sampler2D worldPositionTexture;\n"
"\n"
"uniform vec3 sun;\n"
"uniform vec3 fogFalloff;\n"
"uniform vec2 inverseScreenSize;\n"
"\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   vec2 tex = gl_FragCoord.xy*inverseScreenSize;\n"
"   vec4 diffuse = texture(diffuseTexture, tex);\n"
"   vec4 worldNormal = texture(worldNormalTexture, tex);\n"
"   vec4 worldPosition = texture(worldPositionTexture, tex);\n"
"   \n"
"   float depth = worldPosition.w;\n"
"   float directLight = 0.5f + 0.5f*max(dot(worldNormal.xyz, sun), 0.0f);\n"
"   vec3 decay = vec3(exp(depth*fogFalloff));\n"
"   \n"
"   colour = vec4(diffuse.xyz*directLight*decay + (vec3(1.0f) - decay)*vec3(1.0f), 1.0f);\n"
"   //colour = vec4(diffuse.xyz, 1.0f);\n"
"   //colour = vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
"}\n");
}

void SimpleFogEffect::setSun(const vec3 &sun)
{
    uniformMap.setVec3Uniform(normalize(sun), "sun");
}

void SimpleFogEffect::setFog(const float &fogIntensity)
{
    uniformMap.setVec3Uniform(-fogIntensity*vec3(5.8e-6 + 2.0e-5, 13.5e-6 + 2.0e-5, 33.1e-6 + 2.0e-5), "fogFalloff");
}

SimpleFogEffect *fogEffect = 0;

void setup()
{
    aspectRatio = static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight());
    
    //testMesh = new draw::StaticMesh(mesh::io::readStaticMeshOBJ(DATA_DIRECTORY + "mesh/sponza/sponza_triangles.obj"));
    //testMesh = new draw::StaticMesh(mesh::io::readStaticMeshOBJ(DATA_DIRECTORY + "mesh/sibenik/sibenik_triangles.obj"));
    testMesh = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(4.0f));
    testDiffuseTexture = new draw::RGBATexture2D(img::io::readImage(DATA_DIRECTORY + "img/default.png"));
    testMesh->setDiffuseTexture(*testDiffuseTexture);
    
    terrain = new draw::Terrain(4, 8);
    
    const float lightSpacing = 4.0f;
    
    for (int i = -4; i <= 4; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            pointLightInstances.push_back(draw::PointLightInstance(vec4(lightSpacing*i, 0.0f, lightSpacing*j, 1.0f), vec4(1.0f, 0.9f, 0.3f, lightSpacing)));
        }
    }
    
    pointLights = new draw::PointLightHorde(pointLightInstances.size());
    
    fontTexture = new draw::IconTexture2D(512, 512);
    fontTexture->packIcons(img::io::readFont(DATA_DIRECTORY + "font/OpenBaskerville-0.0.75.ttf", 48));
    
    font = new draw::ScreenIconHorde(1024);
    font->setIconTexture(*fontTexture);
    font->setText(-1.0, -1.0, 0.1, aspectRatio, "The \\rtiny\\w-\\ggame\\w-\\bengine\\w.\nA model rendering example.", *fontTexture);
    
    worldFont = new draw::WorldIconHorde(1024);
    worldFont->setIconTexture(*fontTexture);
    worldFont->setText(0.0, 0.0, 1.0, "The \\rtiny\\w-\\ggame\\w-\\bengine\\w.\nA model rendering example.", *fontTexture);
    
    fogEffect = new SimpleFogEffect();
    
    diffuseTexture = new draw::RGBATexture2D(application->getScreenWidth(), application->getScreenHeight());
    worldNormalTexture = new draw::Vec4Texture2D(application->getScreenWidth(), application->getScreenHeight());
    worldPositionTexture = new draw::Vec4Texture2D(application->getScreenWidth(), application->getScreenHeight());
    depthTexture = new draw::DepthTexture2D(application->getScreenWidth(), application->getScreenHeight());
    
    worldRenderer = new draw::WorldRenderer(aspectRatio);
    worldRenderer->setDiffuseTarget(*diffuseTexture);
    worldRenderer->setNormalsTarget(*worldNormalTexture);
    worldRenderer->setPositionsTarget(*worldPositionTexture);
    worldRenderer->setDepthTextureTarget(*depthTexture);
    
    effectRenderer = new draw::WorldEffectRenderer(aspectRatio);
    effectRenderer->setDiffuseSource(*diffuseTexture);
    effectRenderer->setNormalsSource(*worldNormalTexture);
    effectRenderer->setPositionsSource(*worldPositionTexture);
    
    worldRenderer->addRenderable(testMesh);
    worldRenderer->addRenderable(terrain);
    worldRenderer->addRenderable(worldFont);
    
    effectRenderer->addRenderable(fogEffect, false, false);
    effectRenderer->addRenderable(pointLights, false, false, draw::BlendAdd);
    effectRenderer->addRenderable(font, false, false, draw::BlendMix);
}

void cleanup()
{
    delete effectRenderer;
    delete worldRenderer;
    
    delete depthTexture;
    delete worldPositionTexture;
    delete worldNormalTexture;
    delete diffuseTexture;
    
    delete fogEffect;
    
    delete worldFont;
    delete font;
    delete fontTexture;
    
    delete pointLights;
    
    delete terrain;
    delete testMesh;
    delete testDiffuseTexture;
}

void update(const double &dt)
{
    const float ds = (application->isKeyPressed('f') ? 300.0f : 2.0f)*dt;
    const float dr = 2.1f*dt;
    
    if (application->isKeyPressed('i')) cameraOrientation = quatmul(quatrot(dr, vec3(-1.0f, 0.0f, 0.0f)), cameraOrientation);
    if (application->isKeyPressed('k')) cameraOrientation = quatmul(quatrot(dr, vec3( 1.0f, 0.0f, 0.0f)), cameraOrientation);
    if (application->isKeyPressed('j')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f,-1.0f, 0.0f)), cameraOrientation);
    if (application->isKeyPressed('l')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 1.0f, 0.0f)), cameraOrientation);
    if (application->isKeyPressed('u')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f,-1.0f)), cameraOrientation);
    if (application->isKeyPressed('o')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f, 1.0f)), cameraOrientation);

    quatnormalize(cameraOrientation);

    vec3 vel = mat4(cameraOrientation)*vec3((application->isKeyPressed('d') && application->isKeyPressed('a')) ? 0.0f : (application->isKeyPressed('d') ? 1.0f : (application->isKeyPressed('a') ? -1.0f : 0.0f)),
                                            (application->isKeyPressed('q') && application->isKeyPressed('e')) ? 0.0f : (application->isKeyPressed('q') ? 1.0f : (application->isKeyPressed('e') ? -1.0f : 0.0f)),
                                            (application->isKeyPressed('s') && application->isKeyPressed('w')) ? 0.0f : (application->isKeyPressed('s') ? 1.0f : (application->isKeyPressed('w') ? -1.0f : 0.0f)));
    
    cameraPosition += ds*normalize(vel);
    
    terrain->setCameraPosition(cameraPosition);
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
    effectRenderer->setCamera(cameraPosition, cameraOrientation);
    
    //fogEffect->setSun(vec3(cos(0.5*globalTime), 0.0, sin(0.5*globalTime)));
    //fogEffect->setFog(1024.0 + 1024.0*sin(0.7*globalTime));
    
    for (std::vector<draw::PointLightInstance>::iterator i = pointLightInstances.begin(); i != pointLightInstances.end(); ++i)
    {
        i->position += randomVec4(2.0f*dt);
    }
    
    pointLights->setLights(pointLightInstances.begin(), pointLightInstances.end());
    
    globalTime += dt;
}

void render()
{
    worldRenderer->clearTargets();
    worldRenderer->render();
    //effectRenderer->clearTargets();
    effectRenderer->render();
}

int main(int, char **)
{
    try
    {
        application = new os::SDLApplication(1280, 800);
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

