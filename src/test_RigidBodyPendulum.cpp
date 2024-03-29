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

class GravitySystem : public rigid::RigidBodySystem
{
    public:
        bool pull;
        float E0;
        const float g = 9.81f;

        GravitySystem() : RigidBodySystem()
        {
            const int nrBodies = 8;
            const int nrSpheresPerBody = 8;
            const int nrSpheresPerChain = 8;
            const float radius = 0.01f;
            const float height = 0.2f;
            const float radiusChain = 0.5f*height/static_cast<float>(nrSpheresPerChain);

            std::vector<vec4> shape;

            for (int i = 0; i < nrSpheresPerBody; ++i) shape.push_back(vec4(0.0f, -2.0f*radius*(static_cast<float>(i) + 0.5f), 0.0f, radius));

            for (int i = 0; i < nrBodies; ++i) {
                const float x = 0.5f*height*static_cast<float>(i - nrBodies/2);
                std::vector<int> spheres;

                spheres.push_back(addSpheresRigidBody(0.05f, shape, vec3(x, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), 0.1f, 0.1f, 0.95f, 0.0f));
                for (int j = 0; j < nrSpheresPerChain; ++j) spheres.push_back(addSpheresRigidBody(0.025f/static_cast<float>(nrSpheresPerChain), {vec4(0.0f, 0.0f, 0.0f, radiusChain)}, vec3(x, (2.0f*static_cast<float>(j) + 1.0f)*radiusChain, 0.0f)));
                spheres.push_back(addImmovableSpheresRigidBody({vec4(0.0f, 0.0f, 0.0f, radius)}, vec3(x, height + radius, 0.0f)));

                addPositionConstraint(spheres[0], vec3(0.0f, static_cast<float>(nrSpheresPerBody)*radius, 0.0f), spheres[1], vec3(0.0f, -radiusChain, 0.0f));
                for (int j = 0; j < nrSpheresPerChain - 1; ++j) addPositionConstraint(spheres[j + 1], vec3(0.0f, radiusChain, 0.0f), spheres[j + 2], vec3(0.0f, -radiusChain, 0.0f));
                addPositionConstraint(spheres[nrSpheresPerChain], vec3(0.0f, radiusChain, 0.0f), spheres[nrSpheresPerChain + 1], vec3(0.0f, -radius, 0.0f));

                for (int j = 0; j <= nrSpheresPerChain; ++j) bodies[spheres[j + 1]].canCollide = false;
            }

            for (int i = 0; i < 128; ++i) {
                update(0.01f);
            }

            E0 = 0.0f;

            for (const auto &b : bodies)
            {
                if (b.movable)
                {
                    E0 += g*b.x.y/b.invM; //Potential energy due to gravity.
                }
            }

            pull = false;
        }

        ~GravitySystem()
        {

        }

    protected:
        void applyExternalForces()
        {
            for (auto &b : bodies)
            {
                b.f = vec3(0.0f, -g/b.invM, 0.0f); //Gravity.
            }

            if (pull)
            {
                //Is the user pulling on the pendulum?
                bodies[1].f.x -= 1.0f;
            }
        }

        float potentialEnergy() const
        {
            float e = 0.0f;

            for (const auto &b : bodies)
            {
                if (b.movable)
                {
                    e += g*b.x.y/b.invM; //Potential energy due to gravity.
                }
            }

            return e - E0;
        }
};

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::StaticMeshInstance> sphereMeshInstances;
draw::StaticMeshHorde *sphereMeshHorde = 0;
draw::RGBATexture2D *sphereDiffuseTexture = 0;

GravitySystem *rigidBodySystem = 0;
float lastEnergyTime = -10.0f;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 1.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a rigid body scene.
    rigidBodySystem = new GravitySystem();
    
    //Create a sphere mesh and paint it with a texture.
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
    rigidBodySystem->update(dt);

    if (application->isKeyPressed(' ')) rigidBodySystem->pull = true;
    else rigidBodySystem->pull = false;

    if (rigidBodySystem->getTime() > lastEnergyTime + 0.5f)
    {
        lastEnergyTime = rigidBodySystem->getTime();
        std::cerr << *rigidBodySystem;
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

