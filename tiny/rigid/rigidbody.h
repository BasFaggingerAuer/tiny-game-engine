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
#pragma once

#include <vector>
#include <list>
#include <map>

#include <tiny/math/vec.h>
#include <tiny/rigid/aabbtree.h>

//TODO: Decide whether to separate this out for cleaner dependencies.
#include <tiny/draw/staticmeshhorde.h>

namespace tiny
{

namespace rigid
{

class SpatialSphereHasher
{
    public:
        SpatialSphereHasher(const size_t &, const float &);
        ~SpatialSphereHasher();
        
        void hashObjects(const std::vector<vec4> &);
        const std::vector<std::list<size_t>> *getBuckets() const;
        
    private:
        const float invBoxSize;
        std::vector<std::list<size_t>> buckets;
};

enum class RigidBodyGeometry
{
    Spheres,
    Plane
};

struct RigidBody
{
    float invM; //Inverse mass.
    vec3 invI; //Inverse inertia tensor diagonal.
    vec3 x; //Position.
    vec4 q; //Orientation.
    vec3 v; //Linear velocity.
    vec3 w; //Angular velocity.
    vec3 f; //Force accumulator.
    vec3 t; //Torque accumulator.

    bool movable; //Is the object movable at all (default yes).
    RigidBodyGeometry geometry; //Type of object geometry.
    
    //For Spheres geometry.
    float radius; //Size of rigid body.
    float collisionRadius; //With margin for collision detection.
    size_t firstInternalSphere; //Index of first internal sphere.
    size_t lastInternalSphere; //Index of last internal sphere.

    //For Plane geometry.
    vec4 plane;
    
    //TODO: Move to material class.
    float staticFriction; //Static friction coefficient.
    float dynamicFriction; //Dynamic friction coefficient.
    float restitution; //Restitution coefficient for collisions.
    float softness; //Inverse material hardness.
    
    mat3 getI() const
    {
        return mat3::rotationMatrix(q)*mat3::scaleMatrix(1.0f/invI)*mat3::rotationMatrix(quatconj(q));
    }

    mat3 getInvI() const
    {
        return mat3::rotationMatrix(q)*mat3::scaleMatrix(invI)*mat3::rotationMatrix(quatconj(q));
    }

    friend std::ostream & operator << (std::ostream &Out, const RigidBody &b)
    {
        Out << "rigid body (m = " << 1.0f/b.invM << ", I = " << 1.0f/b.invI << ")" << std::endl
            << "    position    = " << b.x << std::endl
            << "    orientation = " << b.q << std::endl
            << "    lin. vel.   = " << b.v << std::endl
            << "    ang. vel.   = " << b.w << std::endl
            << "    force       = " << b.f << std::endl
            << "    torque      = " << b.t << std::endl;

        return Out;
    }
};

struct RigidBodyCollision
{
    size_t b1Index; //Index of the first rigid body.
    size_t b1SphereIndex; //Index of the first body's internal sphere.
    size_t b2Index; //Index of the second rigid body.
    size_t b2SphereIndex; //Index of the second body's internal sphere.
};

struct RigidBodyCollisionGeometry
{
    RigidBody *b1, *b2; //Pointer to bodies.
    float d; //Signed distance of b2 w.r.t. b1 along the collision normal. A collision occurs if d < 0.
    vec3 p, n; //Point and normal of collision. Distance to move b2 w.r.t. b1 to remove the collision.
    vec3 v1, v2; //Velocities at p of both bodies.
};

class RigidBodySystem
{
    public:
        RigidBodySystem(const size_t & = 2003, const float & = 4.0f, const float & = 0.5f, const int & = 16);
        virtual ~RigidBodySystem();
        
        void addInfinitePlaneBody(const vec4 &,
            const float & = 0.6f, const float & = 0.5f, const float & = 0.7f, const float & = 0.0f); //Friction/restitution ~steel/aluminum.

        void addSpheresRigidBody(const float &, const std::vector<vec4> &,
            const vec3 &, const vec3 & = vec3(0.0f, 0.0f, 0.0f),
            const vec4 & = vec4(0.0f, 0.0f, 0.0f, 1.0f), const vec3 & = vec3(0.0f, 0.0f, 0.0f),
            const float & = 0.6f, const float & = 0.5f, const float & = 0.7f, const float & = 0.0f); //Friction/restitution ~steel/aluminum.
        //TODO: Ability to remove rigid bodies.
        
        void update(const float &);
        
        //Ability to export all internal spheres for rendering in a static mesh horde.
        //Container should contain a StaticMeshInstance-compatible type.
        template <typename Container>
        void getInternalSphereStaticMeshes(Container &out) const
        {
            for (const auto &b : bodies)
            {
                if (b.geometry == RigidBodyGeometry::Spheres)
                {
                    const mat3 R = mat3::rotationMatrix(b.q);
                    
                    for (size_t i = b.firstInternalSphere; i < b.lastInternalSphere; ++i)
                    {
                        const vec4 s = bodyInternalSpheres[i];

                        out.push_back(tiny::draw::StaticMeshInstance(vec4(b.x + R*s.xyz(), s.w), b.q));
                    }
                }
            }
        }

        float getTime() const;
        
        friend std::ostream & operator << (std::ostream &Out, const RigidBodySystem &b)
        {
            Out << "Rigid body system: "
                << b.bodies.size() << " rigid bodies"
                << ", t = " << b.time
                << ", E = " << b.totalEnergy
                << ", P = " << b.totalLinearMomentum
                << ", L = " << b.totalAngularMomentum
                << std::endl;

            return Out;
        }

    protected:
        virtual void applyExternalForces();
        virtual float potentialEnergy() const;
    
        float time;
        float totalEnergy;
        vec3 totalLinearMomentum;
        vec3 totalAngularMomentum;

        std::vector<RigidBody> bodies;
        std::vector<size_t> planeBodyIndices;

        const int nrSubSteps;
        SpatialSphereHasher boundingSphereHasher;
        SpatialSphereHasher internalSphereHasher;

    private:
        std::vector<RigidBody> preBodies;
        std::vector<vec4> bodyInternalSpheres;
        std::vector<vec4> collSpheres;

        void calculateInternalSpheres(const RigidBody &, const float &);
        RigidBodyCollisionGeometry getCollisionGeometry(std::vector<RigidBody> &, const RigidBodyCollision &) const noexcept;
        float addMarginToRadius(const float, const float) const;
};

}

}

