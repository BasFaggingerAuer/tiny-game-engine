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
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/worldrenderer.h>

#include <tiny/rigid/rigidbody.h>

using namespace std;
using namespace tiny;

//#define SHOW_COLLISION_MESH

const float heightOffset = 14.0f;
bool runSimulation = false;
bool followCar = false;
bool showCollisionSpheres = false;

class GravitySystem : public rigid::RigidBodySystem
{
    public:
        GravitySystem(const vec2 &a_terrainScale, const draw::FloatTexture2D *a_terrainHeightTexture) :
            RigidBodySystem(),
            terrainScale(a_terrainScale),
            terrainHeightTexture(a_terrainHeightTexture)
        {
            //Set terrain material properties to be extremely rough.
            bodies[0].staticFriction = 0.9f;
            bodies[0].dynamicFriction = 0.7f;
            bodies[0].restitution = 0.2f;
            bodies[0].softness = 1.0e-6f;

            wheelAngle = 0.0f;
            enginePowerFraction = 0.0f;
            
            std::vector<vec4> wheelGeometry = std::vector<vec4>{vec4(0.0f, 0.0f, 0.0f, 0.5f*wheelDiameter)};
            
            //Add wheels.
            wheel1 = addSpheresRigidBody(wheelWeight, wheelGeometry, vec3(-0.5*wheelLocations.x, 0.5f*wheelDiameter, -0.5*wheelLocations.z), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel2 = addSpheresRigidBody(wheelWeight, wheelGeometry, vec3(-0.5*wheelLocations.x, 0.5f*wheelDiameter,  0.5*wheelLocations.z), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel3 = addSpheresRigidBody(wheelWeight, wheelGeometry, vec3( 0.5*wheelLocations.x, 0.5f*wheelDiameter, -0.5*wheelLocations.z), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            wheel4 = addSpheresRigidBody(wheelWeight, wheelGeometry, vec3( 0.5*wheelLocations.x, 0.5f*wheelDiameter,  0.5*wheelLocations.z), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), wheelStaticFriction, wheelDynamicFriction, wheelCOR, wheelSoftness);
            
            //Add body.
            const ivec3 nrBodySpheres(4, 1, 2);
            const float bodySphereDiameter = 0.5f*((carSize.x/static_cast<float>(nrBodySpheres.x)) +
                                                   (carSize.z/static_cast<float>(nrBodySpheres.z)));
            
            bodyGeometry.clear();

            for (int x = 0; x < nrBodySpheres.x; ++x)
            {
                for (int y = 0; y < nrBodySpheres.y; ++y)
                {
                    for (int z = 0; z < nrBodySpheres.z; ++z)
                    {
                        bodyGeometry.push_back(vec4(bodySphereDiameter*(static_cast<float>(x) - 0.5*static_cast<float>(nrBodySpheres.x - 1)),
                                                    bodySphereDiameter*(static_cast<float>(y) - 0.5*static_cast<float>(nrBodySpheres.y - 1)),
                                                    bodySphereDiameter*(static_cast<float>(z) - 0.5*static_cast<float>(nrBodySpheres.z - 1)),
                                                    0.5f*bodySphereDiameter));
                    }
                }
            }

            //FIXME: To avoid the camera clipping through the terrain.
            bodyGeometry.push_back(vec4(0.0f, bodySphereDiameter*0.5f*static_cast<float>(nrBodySpheres.y), 0.0f, 0.5f*bodySphereDiameter));

            body = addSpheresRigidBody(bodyWeight, bodyGeometry,
                                vec3(0.0f, wheelDiameter, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f),
                                0.61f, 0.47f, 0.70f, 0.0f);

            //Add constraints.
            addNonCollidingPair(body, wheel1);
            addNonCollidingPair(body, wheel2);
            addNonCollidingPair(body, wheel3);
            addNonCollidingPair(body, wheel4);

            wheel1Suspension = addPositionLineConstraint(body, vec3(-0.5*wheelLocations.x, -wheelLocations.y, -0.5*wheelLocations.z), vec3(0.0f, cos(suspensionAngle), -sin(suspensionAngle)), wheel1, vec3(0.0f, 0.0f, 0.0f));
            wheel2Suspension = addPositionLineConstraint(body, vec3(-0.5*wheelLocations.x, -wheelLocations.y,  0.5*wheelLocations.z), vec3(0.0f, cos(suspensionAngle),  sin(suspensionAngle)), wheel2, vec3(0.0f, 0.0f, 0.0f));
            wheel3Suspension = addPositionLineConstraint(body, vec3( 0.5*wheelLocations.x, -wheelLocations.y, -0.5*wheelLocations.z), vec3(0.0f, cos(suspensionAngle), -sin(suspensionAngle)), wheel3, vec3(0.0f, 0.0f, 0.0f));
            wheel4Suspension = addPositionLineConstraint(body, vec3( 0.5*wheelLocations.x, -wheelLocations.y,  0.5*wheelLocations.z), vec3(0.0f, cos(suspensionAngle),  sin(suspensionAngle)), wheel4, vec3(0.0f, 0.0f, 0.0f));
            
            wheel1Steering = addAngularConstraint(body, vec3(0.0f, 0.0f, -1.0f), wheel1, vec3(0.0f, 0.0f, 1.0f));
            wheel2Steering = addAngularConstraint(body, vec3(0.0f, 0.0f,  1.0f), wheel2, vec3(0.0f, 0.0f, 1.0f));
            wheel3Steering = addAngularConstraint(body, vec3(0.0f, 0.0f, -1.0f), wheel3, vec3(0.0f, 0.0f, 1.0f));
            wheel4Steering = addAngularConstraint(body, vec3(0.0f, 0.0f,  1.0f), wheel4, vec3(0.0f, 0.0f, 1.0f));

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
                    }, randomVec3()*vec3(2.0f, 0.0f, 2.0f) - vec3(1.0f, -2*i - 10, 1.0f), vec3(0.0f, 0.0f, 0.0f), normalize(randomVec4() - vec4(0.5f)));
            }

            //Move bodies above the terrain.
            for (size_t i = 1; i < bodies.size(); ++i)
            {
                bodies[i].x.y += heightOffset;
            }
        }

        ~GravitySystem()
        {

        }

        float wheelAngle;
        float enginePowerFraction;

        //Car geometry.
        const float wheelWeight = 40.0f; //kg
        const float wheelDiameter = 1.2f; //0.94f; //m
        const float wheelStaticFriction = 0.9f; //Dry rubber on concrete.
        const float wheelDynamicFriction = 0.7f; //Dry rubber on concrete.
        const float wheelCOR = 0.95f;
        const float wheelSoftness = 0.0f; //1.0e-6f;
        const float suspensionAngle = 0.0f; // 0.01f; //rad, negative camber angle.
        const float suspensionSpringFreeLength = 0.3f; //0.40f; //m
        const float suspensionSpringCoefficient = 64000.0f; //N/m
        const float suspensionDampingCoefficient = 4000.0f; //N/m/s
        const float bodyWeight = 3000.0f; //kg
        const float engineTorque = 700.0f; //N*m
        const vec3 carSize =  vec3(4.8f, 1.6f, 2.3f); //vec3(4.84, 1.83, 2.18); //m
        const vec3 wheelLocations = vec3(3.3f, 0.3f*wheelDiameter, 2.3f); //vec3(3.3f, 0.4f*wheelDiameter, 1.82); //m
        
        const float gravityAcceleration = 9.81f;
        
        int wheel1, wheel2, wheel3, wheel4;
        int body;
        std::vector<vec4> bodyGeometry;
        int wheel1Suspension, wheel2Suspension, wheel3Suspension, wheel4Suspension;
        int wheel1Steering, wheel2Steering, wheel3Steering, wheel4Steering;

        vec2 terrainScale;
        const draw::FloatTexture2D *terrainHeightTexture;

        mesh::StaticMesh terrainCollisionMesh;
    
    protected:
        void applyExternalForces()
        {
            for (auto &b : bodies)
            {
                b.f = vec3(0.0f, -gravityAcceleration/b.invM, 0.0f); //Gravity.
            }

            //TODO: Let engine torque depend on wheel RPM.
            for (auto i : {wheel1, wheel2, wheel3, wheel4})
            {
                bodies[i].t = mat3::rotationMatrix(bodies[body].q)*vec3(0.0f, 0.0f, -enginePowerFraction*engineTorque);
            }
            
            //Springs and damping for the wheels.
            for (const auto &c : {constraints[wheel1Suspension],
                                  constraints[wheel2Suspension],
                                  constraints[wheel3Suspension],
                                  constraints[wheel4Suspension]})
            {
                rigid::RigidBody *b1 = &bodies[c.b1.i];
                rigid::RigidBody *b2 = &bodies[c.b2.i];
                const vec3 p1 = mat3::rotationMatrix(b1->q)*c.b1.r + b1->x;
                const vec3 v1 = b1->v + cross(b1->w, p1 - b1->x);
                const vec3 n = mat3::rotationMatrix(b1->q)*c.n;
                const vec3 p2 = mat3::rotationMatrix(b2->q)*c.b2.r + b2->x;
                const vec3 v2 = b2->v + cross(b2->w, p2 - b2->x);
                
                const vec3 f = -suspensionSpringCoefficient*(dot(p2 - p1, n) + suspensionSpringFreeLength)*n - suspensionDampingCoefficient*dot(v2 - v1, n)*n;

                b1->f -= f;
                b1->t -= cross(p1 - b1->x, f);
                b2->f += f;
                b2->t += cross(p2 - b2->x, f);
            }

            if (true)
            {
                //Direction of the wheels. (Ensure that both wheels are oriented correctly w.r.t. center of the circle the car is making.)
                const float w = 0.5*wheelLocations.z/wheelLocations.x;
                const float c = cos(wheelAngle);
                const float s = sin(wheelAngle);
                float d;

                d = sqrtf(1.0f + 2.0f*w*s*c + w*w*s*s);
                constraints[wheel3Steering].b1.r = vec3( s/d, 0.0f, -(c + w*s)/d);
                d = sqrtf(1.0f - 2.0f*w*s*c + w*w*s*s);
                constraints[wheel4Steering].b1.r = vec3(-s/d, 0.0f,  (c - w*s)/d);
            }
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

            //TODO: Incorporate springs etc.

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

std::vector<draw::StaticMeshInstance> wheelMeshInstances;
draw::StaticMeshHorde *wheelMeshHorde = 0;
draw::RGBTexture2D *wheelDiffuseTexture = 0;
draw::RGBTexture2D *wheelNormalTexture = 0;

std::vector<draw::StaticMeshInstance> bodyMeshInstances;
draw::StaticMeshHorde *bodyMeshHorde = 0;
draw::RGBTexture2D *bodyDiffuseTexture = 0;
draw::RGBTexture2D *bodyNormalTexture = 0;

draw::StaticMesh *terrainCollisionMesh = 0;
draw::RGBATexture2D *terrainCollisionMeshDiffuseTexture = 0;

draw::RGBATexture2D *skyGradientTexture = 0;

GravitySystem *rigidBodySystem = 0;
float lastEnergyTime = -10.0f;

#ifdef SHOW_COLLISION_MESH
draw::Renderable *screenEffect = 0;
#else
draw::StaticMesh *skyBox = 0;
draw::effects::SunSky *screenEffect = 0;
float sunAngle = 1.0f;
#endif

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

    //Load quad meshes.
    wheelMeshHorde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/quadwheel.dae"), 16);
    wheelDiffuseTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/quadwheel.png").flipUpDown());
    wheelNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    wheelMeshHorde->setDiffuseTexture(*wheelDiffuseTexture);
    wheelMeshHorde->setNormalTexture(*wheelNormalTexture);

    bodyMeshHorde = new draw::StaticMeshHorde(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/quadbody.dae"), 16);
    bodyDiffuseTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/quadbody.png").flipUpDown());
    bodyNormalTexture = new draw::RGBTexture2D(img::Image::createUpNormalImage());
    bodyMeshHorde->setDiffuseTexture(*bodyDiffuseTexture);
    bodyMeshHorde->setNormalTexture(*bodyNormalTexture);

    terrainCollisionMeshDiffuseTexture = new draw::RGBATexture2D(img::Image::createSolidImage(4, 255, 255, 0));
    
    //Create simple example terrain.
    terrain = new draw::Terrain(4, 6);
    terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(DATA_DIRECTORY + "img/512hills.png"), draw::tf::filter);
    terrainTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    terrainLocalDiffuseTextures = new draw::RGBTexture2DArray(img::io::readImage(DATA_DIRECTORY + "img/terrain/desert_sand_smooth_b.png"));
    terrainLocalNormalTextures = new draw::RGBTexture2DArray(img::io::readImage(DATA_DIRECTORY + "img/terrain/desert_sand_smooth_b_norm.png"));
    
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
    //screenEffect = new draw::effects::Lambert();
    skyBox = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
    screenEffect = new draw::effects::SunSky();
    skyGradientTexture = new draw::RGBATexture2D(img::io::readImage(DATA_DIRECTORY + "img/sky.png"));
    screenEffect->setSkyTexture(*skyGradientTexture);
    screenEffect->setSun(vec3(sin(sunAngle), cos(sunAngle), 0.5f));
#endif
    
    //Create a renderer and add the cube and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
#ifndef SHOW_COLLISION_MESH
    worldRenderer->addWorldRenderable(0, terrain);
    worldRenderer->addWorldRenderable(1, skyBox);
#endif
    worldRenderer->addWorldRenderable(2, sphereMeshHorde);
    worldRenderer->addWorldRenderable(3, wheelMeshHorde);
    worldRenderer->addWorldRenderable(4, bodyMeshHorde);
    worldRenderer->addScreenRenderable(0, screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    if (skyGradientTexture) delete skyGradientTexture;
    if (skyBox) delete skyBox;
    
    delete sphereMeshHorde;
    delete sphereDiffuseTexture;
    delete sphereNormalTexture;

    delete wheelMeshHorde;
    delete wheelDiffuseTexture;
    delete wheelNormalTexture;

    delete bodyMeshHorde;
    delete bodyDiffuseTexture;
    delete bodyNormalTexture;

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
    rigidBodySystem->wheelAngle = 0.0f;
    if (application->isKeyPressed('3')) rigidBodySystem->wheelAngle =  0.25f;
    if (application->isKeyPressed('1')) rigidBodySystem->wheelAngle = -0.25f;

    rigidBodySystem->enginePowerFraction = 0.0f;
    if (application->isKeyPressed('2')) rigidBodySystem->enginePowerFraction = 4.0f;
    if (application->isKeyPressed('4')) rigidBodySystem->enginePowerFraction = -4.0f;
    
    //Update the rigid bodies.
    if (application->isKeyPressedOnce('r')) runSimulation = !runSimulation;
    if (application->isKeyPressedOnce('t')) showCollisionSpheres = !showCollisionSpheres;
    if (application->isKeyPressedOnce('c')) followCar = !followCar;
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

    if (rigidBodySystem->getTime() > lastEnergyTime + 0.5f)
    {
        lastEnergyTime = rigidBodySystem->getTime();
        std::cerr << *rigidBodySystem;
    }
    
    //Get rigid body positions and send them to the static mesh horde to show the collision spheres.
    sphereMeshInstances.clear();
    if (showCollisionSpheres) rigidBodySystem->getInternalSphereStaticMeshes(sphereMeshInstances);
    sphereMeshHorde->setMeshes(sphereMeshInstances.begin(), sphereMeshInstances.end());

    //Get wheel rigid body positions.
    wheelMeshInstances.clear();
    rigidBodySystem->getStaticMeshes(wheelMeshInstances, std::array<int, 4>({rigidBodySystem->wheel1, rigidBodySystem->wheel2, rigidBodySystem->wheel3, rigidBodySystem->wheel4}));
    wheelMeshHorde->setMeshes(wheelMeshInstances.begin(), wheelMeshInstances.end());

    //Get body position.
    bodyMeshInstances.clear();
    rigidBodySystem->getStaticMeshes(bodyMeshInstances, std::array<int, 1>({rigidBodySystem->body}));
    bodyMeshHorde->setMeshes(bodyMeshInstances.begin(), bodyMeshInstances.end());

    if (followCar)
    {
        //Attach camera to the car.
        vec3 x = cameraPosition;
        vec4 q = quatconj(cameraOrientation);
        
        rigidBodySystem->getRigidBodyPositionAndOrientation(rigidBodySystem->body, x, q);

        cameraPosition = x + mat3::rotationMatrix(q)*rigidBodySystem->bodyGeometry.back().xyz();
        cameraOrientation = quatmul(quatrot(M_PI/2.0f, vec3(0.0f, 1.0f, 0.0f)), quatconj(q));
    }
    else
    {
        //Move the camera around.
        application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);

#ifndef SHOW_COLLISION_MESH
        //If the camera is below the terrain, increase its height.
        const float terrainHeight = sampleTextureBilinear(*terrainHeightTexture, terrainScale, vec2(cameraPosition.x, cameraPosition.z)).x + 1.0f;
        
        cameraPosition.y = std::max(cameraPosition.y, terrainHeight);
#endif
    }

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

