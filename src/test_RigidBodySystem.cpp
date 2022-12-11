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
        GravitySystem() : RigidBodySystem()
        {
            wheelTorques.fill(0.0f);
            
            //Add ground plane.
            addInfinitePlaneBody(vec4(0.0f, 1.0f, 0.0f, 0.0f));
            
            //Create a box.
            /*
            addInfinitePlaneBody(vec4( 1.0f, 0.0f, 0.0f, -4.0f));
            addInfinitePlaneBody(vec4(-1.0f, 0.0f, 0.0f, -4.0f));
            addInfinitePlaneBody(vec4( 0.0f, 0.0f, 1.0f, -4.0f));
            addInfinitePlaneBody(vec4( 0.0f, 0.0f,-1.0f, -4.0f));
            */
            
            //Create wheel geometry.
            std::vector<vec4> wheelGeometry;
            const float wheelRadius = 0.6f;
            const int nrWheelSpheres = 8;
            const float wheelStaticFriction = 1.0f; //Dry rubber on cement.
            const float wheelDynamicFriction = 0.7f; //Dry rubber on cement.
            const float wheelCOR = 0.95f;

            for (int i = 0; i < nrWheelSpheres; ++i)
            {
                const float a = 2.0f*M_PI*static_cast<float>(i)/static_cast<float>(nrWheelSpheres);

                wheelGeometry.push_back(vec4(wheelRadius*cos(a), wheelRadius*sin(a), 0.0f, 1.2f*M_PI*wheelRadius/static_cast<float>(nrWheelSpheres)));
            }

            wheelGeometry = std::vector<vec4>{vec4(0.0f, 0.0f, 0.0f, 1.4f*wheelRadius)};
            
            //Add wheels.
            wheel1 = addSpheresRigidBody(40.0f, wheelGeometry, vec3(-1.6f, 2.0f*wheelRadius, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR);
            wheel2 = addSpheresRigidBody(40.0f, wheelGeometry, vec3(-1.6f, 2.0f*wheelRadius,  1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR);
            wheel3 = addSpheresRigidBody(40.0f, wheelGeometry, vec3( 1.6f, 2.0f*wheelRadius, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR);
            wheel4 = addSpheresRigidBody(40.0f, wheelGeometry, vec3( 1.6f, 2.0f*wheelRadius,  1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR);
            
            //Add body.
            const float bodySphereRadius = 1.0f;

            body = addSpheresRigidBody(3000.0f,
                                {vec4(-2.0f, 0.0f, 0.0f, bodySphereRadius),
                                 vec4( 0.0f, 0.0f, 0.0f, bodySphereRadius),
                                 vec4( 2.0f, 0.0f, 0.0f, bodySphereRadius)},
                                vec3(0.0f, bodySphereRadius + 1.5f*wheelRadius, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f),
                                0.61f, 0.47f, 0.70f);

            //Add constraints.
            addNonCollidingPair(body, wheel1);
            addNonCollidingPair(body, wheel2);
            addNonCollidingPair(body, wheel3);
            addNonCollidingPair(body, wheel4);
            addPositionConstraint(body, vec3(-1.6f, -0.5f*wheelRadius, -1.0f), wheel1, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3(-1.6f, -0.5f*wheelRadius,  1.0f), wheel2, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3( 1.6f, -0.5f*wheelRadius, -1.0f), wheel3, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3( 1.6f, -0.5f*wheelRadius,  1.0f), wheel4, vec3(0.0f, 0.0f, 0.0f));
            addAngularConstraint(body, vec3(0.0f, 0.0f, -1.0f), wheel1, vec3(0.0f, 0.0f, 1.0f));
            addAngularConstraint(body, vec3(0.0f, 0.0f,  1.0f), wheel2, vec3(0.0f, 0.0f, 1.0f));
            addAngularConstraint(body, vec3(0.0f, 0.0f, -1.0f), wheel3, vec3(0.0f, 0.0f, 1.0f));
            addAngularConstraint(body, vec3(0.0f, 0.0f,  1.0f), wheel4, vec3(0.0f, 0.0f, 1.0f));

            //Add something to drive against.
            addSpheresRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 1.0f),
                                       vec4(0.0f, 0.0f, 1.0f, 1.0f),
                                       vec4(0.0f, 0.0f, 2.0f, 1.0f),
                                       vec4(0.0f, 0.0f, 3.0f, 1.0f)},
                                vec3(-10.0f, 2.0f, 0.0f));
            addSpheresRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 1.0f),
                                       vec4(1.0f, 0.0f, 1.0f, 1.0f),
                                       vec4(2.0f, 0.0f, 2.0f, 1.0f),
                                       vec4(3.0f, 0.0f, 3.0f, 1.0f)},
                                vec3(-10.0f, 4.0f, 0.0f));
            
            /*
            //Add some rigid bodies.
            for (int i = 0; i < 64; ++i)
            {
                addSpheresRigidBody(1.0f, {
                    vec4(0.0f, 0.0f, 0.0f, 0.3f),
                    vec4(0.3f, 0.0f, 0.0f, 0.3f),
                    vec4(0.6f, 0.0f, 0.0f, 0.3f),
                    vec4(0.0f, 0.3f, 0.0f, 0.3f),
                    vec4(0.0f, 0.6f, 0.0f, 0.3f)
                    }, randomVec3()*vec3(2.0f, 0.0f, 2.0f) - vec3(1.0f, -2*i - 1, 1.0f), vec3(0.0f, 0.0f, 0.0f), normalize(randomVec4() - vec4(0.5f)));
            }
            */
            
            //addSpheresRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 1.0f)}, vec3(0.0f, 0.5f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
            
            /*
            for (int i = 0; i < 4; ++i)
            {
                addSpheresRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 0.3f)}, vec3(i, 1.0f, 0.0f));
            }
            
            addSpheresRigidBody(1.0f, {vec4(0.0f, 0.0f, 0.0f, 0.3f)}, vec3(-1.0f, 0.9f, 0.0f), vec3(1.0f, 0.0f, 0.0f));
            */
            
            /*
            //Add some rigid bodies.
            addSpheresRigidBody(1.0f, {
                vec4(0.0f, 0.0f, 0.0f, 0.3f),
                vec4(0.3f, 0.0f, 0.0f, 0.3f),
                vec4(0.6f, 0.0f, 0.0f, 0.3f),
                vec4(0.0f, 0.3f, 0.0f, 0.3f),
                vec4(0.0f, 0.6f, 0.0f, 0.3f)
                }, vec3(0.0f, 3.0f, 0.0f));

            addSpheresRigidBody(1.0f, {
                vec4(0.0f, 0.0f, 0.0f, 0.3f),
                vec4(0.3f, 0.0f, 0.0f, 0.3f),
                vec4(0.6f, 0.0f, 0.0f, 0.3f),
                vec4(0.0f, 0.3f, 0.0f, 0.3f),
                vec4(0.0f, 0.6f, 0.0f, 0.3f)
                }, vec3(-2.0f, 2.0f, 0.0f), vec3(2.0f, 0.0f, 0.0f));
             */
        }

        ~GravitySystem()
        {

        }

        std::array<float, 4> wheelTorques;
        int wheel1, wheel2, wheel3, wheel4;
        int body;
    
    protected:
        void applyExternalForces()
        {
            for (auto &b : bodies)
            {
                b.f = vec3(0.0f, -9.81f/b.invM, 0.0f); //Gravity.
            }

            bodies[wheel1].t = vec3(0.0f, 0.0f, wheelTorques[0]);
            bodies[wheel2].t = vec3(0.0f, 0.0f, wheelTorques[1]);
            bodies[wheel3].t = vec3(0.0f, 0.0f, wheelTorques[2]);
            bodies[wheel4].t = vec3(0.0f, 0.0f, wheelTorques[3]);
            //bodies[body].f = vec3(1.0e5f, 0.0f, 0.0f);
        }

        float potentialEnergy() const
        {
            float e = 0.0f;

            for (const auto &b : bodies)
            {
                if (b.movable)
                {
                    e += 9.81f*b.x.y/b.invM; //Potential energy due to gravity.
                }
            }

            return e;
        }
};

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::StaticMeshInstance> sphereMeshInstances;
std::vector<draw::StaticMeshInstance> planeMeshInstances;
draw::StaticMeshHorde *sphereMeshHorde = 0;
draw::StaticMeshHorde *planeMeshHorde = 0;
draw::RGBATexture2D *sphereDiffuseTexture = 0;

GravitySystem *rigidBodySystem = 0;
bool projectVelocities = true;
float lastEnergyTime = -10.0f;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 2.0f, 10.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a rigid body scene.
    rigidBodySystem = new GravitySystem();
    
    //Create a sphere mesh and paint it with a texture.
    sphereMeshHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createIcosahedronMesh(1.0f), 1024);
    sphereDiffuseTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    sphereMeshHorde->setDiffuseTexture(*sphereDiffuseTexture);
    planeMeshHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createPlaneMesh(10.0f), 16);
    planeMeshHorde->setDiffuseTexture(*sphereDiffuseTexture);
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(0, sphereMeshHorde);
    worldRenderer->addWorldRenderable(1, planeMeshHorde);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete planeMeshHorde;
    delete sphereMeshHorde;
    delete sphereDiffuseTexture;
    
    delete rigidBodySystem;
}

void update(const double &dt)
{
    //Control wheels.
    const float tq = 1000.0f;

    if (application->isKeyPressedOnce('1')) rigidBodySystem->wheelTorques[0] =  tq;
    if (application->isKeyPressedOnce('2')) rigidBodySystem->wheelTorques[0] = -tq;
    if (application->isKeyPressedOnce('3')) rigidBodySystem->wheelTorques[1] =  tq;
    if (application->isKeyPressedOnce('4')) rigidBodySystem->wheelTorques[1] = -tq;
    if (application->isKeyPressedOnce('5')) rigidBodySystem->wheelTorques[2] =  tq;
    if (application->isKeyPressedOnce('6')) rigidBodySystem->wheelTorques[2] = -tq;
    if (application->isKeyPressedOnce('7')) rigidBodySystem->wheelTorques[3] =  tq;
    if (application->isKeyPressedOnce('8')) rigidBodySystem->wheelTorques[3] = -tq;

    //Update the rigid bodies.
    //if (application->isKeyPressedOnce(' '))
    //{
        rigidBodySystem->update(dt, projectVelocities);
    //}

    if (application->isKeyPressedOnce('p'))
    {
        projectVelocities = !projectVelocities;
        std::cerr << "Projecting velocities: " << projectVelocities << std::endl;
    }

    if (rigidBodySystem->getTime() > lastEnergyTime + 0.5f)
    {
        lastEnergyTime = rigidBodySystem->getTime();
        std::cerr << *rigidBodySystem;
    }
    
    //Get rigid body positions and send them to the static mesh horde.
    sphereMeshInstances.clear();
    rigidBodySystem->getInternalSphereStaticMeshes(sphereMeshInstances);
    sphereMeshHorde->setMeshes(sphereMeshInstances.begin(), sphereMeshInstances.end());

    //TODO: Link this to the actual planes.
    planeMeshInstances.clear();
    planeMeshInstances.push_back({vec4(0.0f, 0.0f, 0.0f, 1.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f)});
    planeMeshHorde->setMeshes(planeMeshInstances.begin(), planeMeshInstances.end());
    
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

