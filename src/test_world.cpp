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
#include <tiny/draw/computetexture.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/texture2d.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *renderer = 0;
draw::StaticMesh *testMesh = 0;
draw::RGBATexture2D *testDiffuseTexture = 0;
draw::ComputeTexture *screenRenderEffect = 0;

draw::RGBATexture2D *diffuseTexture = 0;
draw::Vec4Texture2D *worldNormalTexture = 0;
draw::Vec4Texture2D *worldPositionTexture = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 0.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    testMesh = new draw::StaticMesh(mesh::io::readStaticMeshOBJ(DATA_DIRECTORY + "mesh/sponza/sponza.obj"));
    testDiffuseTexture = new tiny::draw::RGBATexture2D(tiny::img::io::readImage(DATA_DIRECTORY + "img/test.png"));
    
    testMesh->setDiffuseTexture(*testDiffuseTexture);
    
    diffuseTexture = new draw::RGBATexture2D(application->getScreenWidth(), application->getScreenHeight());
    worldNormalTexture = new draw::Vec4Texture2D(application->getScreenWidth(), application->getScreenHeight());
    worldPositionTexture = new draw::Vec4Texture2D(application->getScreenWidth(), application->getScreenHeight());
    
    renderer = new draw::WorldRenderer();
    renderer->addRenderable(testMesh);
    renderer->setTextureTarget(*diffuseTexture, "diffuse");
    renderer->setTextureTarget(*worldNormalTexture, "worldNormal");
    renderer->setTextureTarget(*worldPositionTexture, "worldPosition");
    
    vector<string> inputTextures;
    vector<string> outputTextures;
    const string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuse;\n"
"uniform sampler2D worldNormal;\n"
"uniform sampler2D worldPosition;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   colour = vec4(texture(diffuse, tex).xyz*exp(-texture(worldPosition, tex).w), 1.0f);\n"
"}\n";
    
    inputTextures.push_back("diffuse");
    inputTextures.push_back("worldNormal");
    inputTextures.push_back("worldPosition");
    outputTextures.push_back("colour");
    
    screenRenderEffect = new tiny::draw::ComputeTexture(inputTextures, outputTextures, fragmentShader);
    screenRenderEffect->setInput(*diffuseTexture, "diffuse");
    screenRenderEffect->setInput(*worldNormalTexture, "worldNormal");
    screenRenderEffect->setInput(*worldPositionTexture, "worldPosition");
}

void cleanup()
{
    delete screenRenderEffect;
    
    delete renderer;
    
    delete worldPositionTexture;
    delete worldNormalTexture;
    delete diffuseTexture;
    
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
    
    renderer->setCamera(cameraPosition, cameraOrientation);
    //cout << cameraPosition << " -- " << cameraOrientation << endl;
}

void render()
{
    glClear(GL_DEPTH_BUFFER_BIT);
    
    renderer->render();
    screenRenderEffect->compute();
}

int main(int, char **)
{
    try
    {
        application = new os::SDLApplication(1024, 768);
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

