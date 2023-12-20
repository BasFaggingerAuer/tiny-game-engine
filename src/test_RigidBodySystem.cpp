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

#include <tiny/img/io/image.h>
#include <tiny/img/image.h>

#include <tiny/mesh/staticmesh.h>
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/draw/texture2darray.h>
#include <tiny/draw/terrain.h>
#include <tiny/draw/heightmap/tangentmap.h>
#include <tiny/draw/heightmap/normalmap.h>
#include <tiny/draw/effects/lambert.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/effects/lambert.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/rigid/rigidbody.h>

using namespace std;
using namespace tiny;

//#define SHOW_COLLISION_MESH

const float heightOffset = 14.0f;
bool runSimulation = false;

class GravitySystem : public rigid::RigidBodySystem
{
    public:
        GravitySystem(const vec2 &a_terrainScale, const draw::FloatTexture2D *a_terrainHeightTexture) :
            RigidBodySystem(),
            terrainScale(a_terrainScale),
            terrainHeightTexture(a_terrainHeightTexture)
        {
            wheelTorques.fill(0.0f);
            
            //Create wheel geometry.
            std::vector<vec4> wheelGeometry;
            const float wheelRadius = 0.7f;
            const float wheelStaticFriction = 1.0f; //Dry rubber on cement.
            const float wheelDynamicFriction = 0.7f; //Dry rubber on cement.
            const float wheelCOR = 0.95f;
            const float wheelSoftness = 1.0e-6f;

            wheelGeometry = std::vector<vec4>{vec4(0.0f, 0.0f, 0.0f, 1.4f*wheelRadius)};
            
            //Add wheels.
            wheel1 = addSpheresRigidBody(40.0f, wheelGeometry, vec3(-1.6f, 2.0f*wheelRadius, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel2 = addSpheresRigidBody(40.0f, wheelGeometry, vec3(-1.6f, 2.0f*wheelRadius,  1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel3 = addSpheresRigidBody(40.0f, wheelGeometry, vec3( 1.6f, 2.0f*wheelRadius, -1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel4 = addSpheresRigidBody(40.0f, wheelGeometry, vec3( 1.6f, 2.0f*wheelRadius,  1.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            
            //Add body.
            const float bodySphereRadius = 1.0f;
            const float bodyHeight = 0.8f*wheelRadius;

            body = addSpheresRigidBody(3000.0f,
                                {vec4(-2.0f, 0.0f, 0.0f, bodySphereRadius),
                                 vec4( 0.0f, 0.0f, 0.0f, bodySphereRadius),
                                 vec4( 2.0f, 0.0f, 0.0f, bodySphereRadius)},
                                vec3(0.0f, bodySphereRadius + wheelRadius + bodyHeight, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f),
                                0.61f, 0.47f, 0.70f, 0.0f);

            //Add constraints.
            addNonCollidingPair(body, wheel1);
            addNonCollidingPair(body, wheel2);
            addNonCollidingPair(body, wheel3);
            addNonCollidingPair(body, wheel4);
            addPositionConstraint(body, vec3(-1.6f, -bodyHeight, -1.0f), wheel1, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3(-1.6f, -bodyHeight,  1.0f), wheel2, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3( 1.6f, -bodyHeight, -1.0f), wheel3, vec3(0.0f, 0.0f, 0.0f));
            addPositionConstraint(body, vec3( 1.6f, -bodyHeight,  1.0f), wheel4, vec3(0.0f, 0.0f, 0.0f));
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
                                vec3(-16.0f, 4.0f, 0.0f));
            
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

            for (size_t i = 1; i < bodies.size(); ++i)
            {
                bodies[i].x.y += heightOffset;
            }
        }

        ~GravitySystem()
        {

        }

        std::array<float, 4> wheelTorques;
        int wheel1, wheel2, wheel3, wheel4;
        int body;

        vec2 terrainScale;
        const draw::FloatTexture2D *terrainHeightTexture;

        mesh::StaticMesh terrainCollisionMesh;
    
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

        std::vector<vec3> getCollisionTriangles(const rigid::RigidBody &b)
        {
            //Turn terrain close to this object into a set of triangles.
            if (!terrainHeightTexture) return std::vector<vec3>();

            const int xLo = std::floor((b.x.x - b.collisionRadius)/terrainScale.x + 0.5f*static_cast<float>(terrainHeightTexture->getWidth()));
            const int yLo = std::floor((b.x.z - b.collisionRadius)/terrainScale.y + 0.5f*static_cast<float>(terrainHeightTexture->getHeight()));
            const int xHi =  std::ceil((b.x.x + b.collisionRadius)/terrainScale.x + 0.5f*static_cast<float>(terrainHeightTexture->getWidth()));
            const int yHi =  std::ceil((b.x.z + b.collisionRadius)/terrainScale.y + 0.5f*static_cast<float>(terrainHeightTexture->getHeight()));
            std::vector<vec3> triangles;

            triangles.reserve((xHi + 1 - xLo)*(yHi + 1 - yLo)*2*3);

            //FIXME: This can be done far more efficiently.
            for (int y = yLo; y <= yHi; ++y)
            {
                for (int x = xLo; x <= xHi; ++x)
                {
                    const float h00 = (*terrainHeightTexture)(x + 0, y + 0).x;
                    const float h01 = (*terrainHeightTexture)(x + 0, y + 1).x;
                    const float h10 = (*terrainHeightTexture)(x + 1, y + 0).x;
                    const float h11 = (*terrainHeightTexture)(x + 1, y + 1).x;

                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h00,
                                             terrainScale.y*(static_cast<float>(y) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));
                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h11,
                                             terrainScale.y*(static_cast<float>(y + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));
                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h10,
                                             terrainScale.y*(static_cast<float>(y) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));
                    
                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h00,
                                             terrainScale.y*(static_cast<float>(y) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));
                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h01,
                                             terrainScale.y*(static_cast<float>(y + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));   
                    triangles.push_back(vec3(terrainScale.x*(static_cast<float>(x + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getWidth())),
                                             h11,
                                             terrainScale.y*(static_cast<float>(y + 1) - 0.5f*static_cast<float>(terrainHeightTexture->getHeight()))));
                }
            }

#ifdef SHOW_COLLISION_MESH
            for (const auto &t : triangles)
            {
                terrainCollisionMesh.indices.push_back(terrainCollisionMesh.vertices.size());
                terrainCollisionMesh.vertices.push_back(mesh::StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), t));
            }
#endif
            
            return triangles;
        }
};

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::StaticMeshInstance> sphereMeshInstances;
draw::StaticMeshHorde *sphereMeshHorde = 0;
draw::RGBATexture2D *sphereDiffuseTexture = 0;
draw::RGBTexture2D *sphereNormalTexture = 0;

draw::StaticMesh *terrainCollisionMesh = 0;
draw::RGBATexture2D *terrainCollisionMeshDiffuseTexture = 0;

GravitySystem *rigidBodySystem = 0;
float lastEnergyTime = -10.0f;

draw::Renderable *screenEffect = 0;

const vec2 terrainScale = vec2(5.0f, 5.0f);
draw::Terrain *terrain = 0;
draw::FloatTexture2D *terrainHeightTexture = 0;
draw::RGBTexture2D *terrainTangentTexture = 0;
draw::RGBTexture2D *terrainNormalTexture = 0;
draw::RGBATexture2D *terrainAttributeTexture = 0;

const vec2 terrainDiffuseScale = vec2(1024.0f, 1024.0f);
draw::RGBTexture2DArray *terrainLocalDiffuseTextures = 0;
draw::RGBTexture2DArray *terrainLocalNormalTextures = 0;

vec3 cameraPosition = vec3(0.0f, heightOffset + 2.0f, 10.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

//A simple bilinear texture sampler, which converts world coordinates to the corresponding texture coordinates on the zoomed-in terrain.
template<typename TextureType>
vec4 sampleTextureBilinear(const TextureType &texture, const vec2 &scale, const vec2 &a_pos)
{
    //Sample texture at the four points surrounding pos.
    const vec2 pos = vec2(a_pos.x/scale.x + 0.5f*static_cast<float>(texture.getWidth()), a_pos.y/scale.y + 0.5f*static_cast<float>(texture.getHeight()));
    const ivec2 intPos = ivec2(static_cast<int>(floor(pos.x)), static_cast<int>(floor(pos.y)));
    const vec4 h00 = texture(intPos.x + 0, intPos.y + 0);
    const vec4 h01 = texture(intPos.x + 0, intPos.y + 1);
    const vec4 h10 = texture(intPos.x + 1, intPos.y + 0);
    const vec4 h11 = texture(intPos.x + 1, intPos.y + 1);
    const vec2 delta = vec2(pos.x - floor(pos.x), pos.y - floor(pos.y));
    
    //Interpolate between these four points.
    return delta.y*(delta.x*h11 + (1.0f - delta.x)*h01) + (1.0f - delta.y)*(delta.x*h10 + (1.0f - delta.x)*h00);
}

void setup()
{
    //Create a sphere mesh and paint it with a texture.
    sphereMeshHorde = new draw::StaticMeshHorde(mesh::StaticMesh::createIcosahedronMesh(1.0f), 1024);
    sphereDiffuseTexture = new draw::RGBATexture2D(img::Image::createSolidImage(4, 0, 255, 255));
    sphereNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    sphereMeshHorde->setDiffuseTexture(*sphereDiffuseTexture);
    sphereMeshHorde->setNormalTexture(*sphereNormalTexture);

    terrainCollisionMeshDiffuseTexture = new draw::RGBATexture2D(img::Image::createSolidImage(4, 255, 255, 0));
    
    //Create simple example terrain.
    terrain = new draw::Terrain(4, 6);
    terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(DATA_DIRECTORY + "img/512hills.png"), draw::tf::filter);
    terrainTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    terrainLocalDiffuseTextures = new draw::RGBTexture2DArray(img::io::readImage(DATA_DIRECTORY + "img/terrain/dirt.jpg"));
    terrainLocalNormalTextures = new draw::RGBTexture2DArray(img::io::readImage(DATA_DIRECTORY + "img/terrain/dirt_normal.jpg"));
    
    //Calculate normal map and paint the terrain with the textures.
    draw::computeTangentMap(*terrainHeightTexture, *terrainTangentTexture, terrainScale.x);
    draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
    terrain->setHeightTextures(*terrainHeightTexture, *terrainTangentTexture, *terrainNormalTexture, terrainScale);
    terrain->setDiffuseTextures(*terrainAttributeTexture, *terrainLocalDiffuseTextures, *terrainLocalNormalTextures, terrainDiffuseScale);

    //Create a rigid body scene.
    rigidBodySystem = new GravitySystem(terrainScale, terrainHeightTexture);

#ifdef SHOW_COLLISION_MESH
    screenEffect = new draw::effects::Diffuse();
#else
    screenEffect = new draw::effects::Lambert();
#endif
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
#ifndef SHOW_COLLISION_MESH
    worldRenderer->addWorldRenderable(0, terrain);
#endif
    worldRenderer->addWorldRenderable(1, sphereMeshHorde);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete sphereMeshHorde;
    delete sphereDiffuseTexture;
    delete sphereNormalTexture;

    if (terrainCollisionMesh) delete terrainCollisionMesh;
    delete terrainCollisionMeshDiffuseTexture;
    
    delete terrain;
    delete terrainHeightTexture;
    delete terrainTangentTexture;
    delete terrainNormalTexture;
    delete terrainAttributeTexture;
    delete terrainLocalDiffuseTextures;
    delete terrainLocalNormalTextures;
    
    delete rigidBodySystem;
}

void update(const double &dt)
{
    //Control wheels.
    const float tq = 10000.0f;

    if (application->isKeyPressed('1')) rigidBodySystem->wheelTorques[0] =  tq;
    if (application->isKeyPressed('2')) rigidBodySystem->wheelTorques[0] = -tq;
    if (application->isKeyPressed('3')) rigidBodySystem->wheelTorques[1] =  tq;
    if (application->isKeyPressed('4')) rigidBodySystem->wheelTorques[1] = -tq;
    if (application->isKeyPressed('5')) rigidBodySystem->wheelTorques[2] =  tq;
    if (application->isKeyPressed('6')) rigidBodySystem->wheelTorques[2] = -tq;
    if (application->isKeyPressed('7')) rigidBodySystem->wheelTorques[3] =  tq;
    if (application->isKeyPressed('8')) rigidBodySystem->wheelTorques[3] = -tq;

    //Update the rigid bodies.
    if (application->isKeyPressedOnce('r')) runSimulation = !runSimulation;
    if (application->isKeyPressedOnce(' ') || runSimulation)
    {
#ifdef SHOW_COLLISION_MESH
        rigidBodySystem->terrainCollisionMesh.vertices.clear();
        rigidBodySystem->terrainCollisionMesh.indices.clear();
#endif

        rigidBodySystem->update(dt);

#ifdef SHOW_COLLISION_MESH
        if (terrainCollisionMesh)
        {
            worldRenderer->freeWorldRenderable(2);
            delete terrainCollisionMesh;
        }

        if (!rigidBodySystem->terrainCollisionMesh.vertices.empty())
        {
            std::cout << rigidBodySystem->terrainCollisionMesh.indices.size()/3 << " collision triangles." << std::endl;
            terrainCollisionMesh = new draw::StaticMesh(rigidBodySystem->terrainCollisionMesh);
            terrainCollisionMesh->setDiffuseTexture(*terrainCollisionMeshDiffuseTexture);
            terrainCollisionMesh->setNormalTexture(*sphereNormalTexture);
            worldRenderer->addWorldRenderable(2, terrainCollisionMesh);
        }
#endif
    }

    for (int i = 0; i < 4; ++i) rigidBodySystem->wheelTorques[i] = 0.0f;

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

#ifndef SHOW_COLLISION_MESH
    //If the camera is below the terrain, increase its height.
    const float terrainHeight = sampleTextureBilinear(*terrainHeightTexture, terrainScale, vec2(cameraPosition.x, cameraPosition.z)).x + 1.0f;
    
    cameraPosition.y = std::max(cameraPosition.y, terrainHeight);
#endif

    //Let the terrain follow the camera.
    terrain->setCameraPosition(cameraPosition);
    
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

