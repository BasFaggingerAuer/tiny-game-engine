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
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/rigid/triangle.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

mesh::StaticMesh triangle;
draw::StaticMesh *triangleMesh = 0;
draw::StaticMeshHorde *icoMeshHorde = 0;
std::vector<draw::StaticMeshInstance> icoMeshInstances;
draw::RGBATexture2D *diffuseTexture = 0;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 3.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a non-trivial triangle.
    triangle.vertices.push_back(tiny::mesh::StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3( 1.0f,  2.0f,  3.0f)));
    triangle.vertices.push_back(tiny::mesh::StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-4.0f, -5.0f, -6.0f)));
    triangle.vertices.push_back(tiny::mesh::StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-7.0f,  8.0f, -9.0f)));
    triangle.indices.push_back(0);
    triangle.indices.push_back(1);
    triangle.indices.push_back(2);
    triangle.indices.push_back(2);
    triangle.indices.push_back(1);
    triangle.indices.push_back(0);
    
    triangleMesh = new draw::StaticMesh(triangle);
    icoMeshHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createIcosahedronMesh(0.125f), 16);
    icoMeshInstances = {draw::StaticMeshInstance(vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f))};
    icoMeshHorde->setMeshes(icoMeshInstances.begin(), icoMeshInstances.end());

    diffuseTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    triangleMesh->setDiffuseTexture(*diffuseTexture);
    icoMeshHorde->setDiffuseTexture(*diffuseTexture);
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(0, triangleMesh);
    worldRenderer->addWorldRenderable(1, icoMeshHorde);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete triangleMesh;
    delete icoMeshHorde;
    delete diffuseTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
    
    //Follow the camera location with the closest point on the triangle mesh.
    const vec3 v = getClosestPointOnTriangle(cameraPosition, {{triangle.vertices[0].position, triangle.vertices[1].position, triangle.vertices[2].position}});
    
    icoMeshInstances = {draw::StaticMeshInstance(vec4(v.x, v.y, v.z, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f))};
    icoMeshHorde->setMeshes(icoMeshInstances.begin(), icoMeshInstances.end());
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

