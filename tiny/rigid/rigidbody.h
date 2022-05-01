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

//TODO: Decide whether to separate this out for cleaner dependencies.
#include <tiny/draw/staticmeshhorde.h>

namespace tiny
{

namespace rigid
{

struct HardSphereInstance
{
    HardSphereInstance()
    {

    }

    HardSphereInstance(const vec3 &_pos, const float &_rad) :
        posAndRadius(vec4(_pos.x, _pos.y, _pos.z, _rad))
    {

    }

    HardSphereInstance(const vec4 &_posAndRadius) :
        posAndRadius(_posAndRadius)
    {

    }

    vec4 posAndRadius;
};

class RigidBody
{
    public:
        RigidBody(const float &, const std::vector<HardSphereInstance> &, const vec3 & = vec3(0.0f, 0.0f, 0.0f), const vec4 & = vec4(0.0f, 0.0f, 0.0f, 1.0f), const vec3 & = vec3(0.0f, 0.0f, 0.0f), const vec3 & = vec3(0.0f, 0.0f, 0.0f));
        RigidBody(const RigidBody &);
        virtual ~RigidBody();

        float getEnergy() const;
        void calculateSphereParameters();
        
        //Rigid body collision detection is performed by decomposing the rigid body into hard spheres and performing only sphere-sphere collisions. This permits us to not consider all vertex/edge/plane collision cases at the cost of increased computation time.
        std::vector<HardSphereInstance> spheres;
        HardSphereInstance boundingSphere;
        
        float inverseMass;
        //TODO: Orient body such that this is a diagonal matrix to make inversion more efficient.
        mat3 inverseInertia;

        vec3 position;
        vec4 orientation;
        vec3 linearMomentum;
        vec3 angularMomentum;
        
};

struct RigidBodyState
{
    RigidBodyState() = default;
    RigidBodyState(const RigidBody &);

    float invM; //Inverse mass.
    mat3 invI; //Inverse inertia tensor.
    vec3 x; //Position.
    vec4 q; //Orientation.
    vec3 v; //Linear velocity.
    vec3 w; //Angular velocity.

    //FIXME: This is horrendous.
    std::vector<HardSphereInstance> spheres;
};

struct RigidBodyCollision
{
    size_t b1Index; //Index of the first rigid body.
    size_t b1SphereIndex; //Index of the first body's internal sphere.
    size_t b2Index; //Index of the second rigid body.
    size_t b2SphereIndex; //Index of the second body's internal sphere.
};

class Constraint
{
    Constraint(const size_t &, const vec3 &,
        const vec3 &,
        const float &, const float & = 0.0f);
    virtual ~Constraint();

    size_t b1Index; //Index of the rigid body.
    vec3 r1; //Relative position of constraint for b1 in global coordinates.
    vec3 n; //Constraint outward normal direction at point of contact in global coordinates.
    float alpha; //Compliance.
    float lambda; //Lagrange multiplier.
};

class TwoBodyConstraint : public Constraint
{
    TwoBodyConstraint(const size_t &, const vec3 &,
        const size_t &, const vec3 &,
        const vec3 &,
        const float &, const float & = 0.0f);
    virtual ~TwoBodyConstraint();

    size_t b2Index; //Index of the second rigid body in the constraint.
    vec3 r2; //Relative position of constraint for b2 in global coordinates.
};

class SpatialSphereHasher
{
    public:
        SpatialSphereHasher(const size_t &, const float &);
        ~SpatialSphereHasher();
        
        void hashObjects(const std::vector<HardSphereInstance> &);
        const std::vector<std::list<size_t>> *getBuckets() const;
        
    private:
        const float invBoxSize;
        std::vector<std::list<size_t>> buckets;
};

class RigidBodySystem
{
    public:
        RigidBodySystem(const size_t & = 4, const float & = 1.0e-6f, const size_t & = 2003, const float & = 4.0f, const float & = 0.5f, const float & = 2.0f);
        virtual ~RigidBodySystem();
        
        void addRigidBody(const unsigned int &, RigidBody *);
        bool freeRigidBody(const unsigned int &);
        
        void update(const float &);
        
        //Ability to export all internal spheres for rendering in a static mesh horde.
        //Container should contain a StaticMeshInstance-compatible type.
        template <typename Container>
        void getInternalSphereStaticMeshes(Container &out) const
        {
            for (std::map<unsigned int, RigidBody *>::const_iterator i = bodies.begin(); i != bodies.end(); ++i)
            {
                //Integrate position and orientation.
                const RigidBody *b = i->second;
                const mat3 R = mat3::rotationMatrix(b->orientation);
            
                for (std::vector<HardSphereInstance>::const_iterator j = b->spheres.begin(); j != b->spheres.end(); ++j)
                {
                    out.push_back(tiny::draw::StaticMeshInstance(vec4(b->position + R*j->posAndRadius.xyz(), j->posAndRadius.w), b->orientation));
                }
            }
        }
        
    protected:
        void integratePositionsAndCalculateBoundingSpheres(const float &);
        void integratePositionsAndCalculateInternalSpheres(const RigidBody *, const float &);
        void calculateInternalSpheres(const RigidBody *);
        virtual void applyExternalForces(const float &);
    
        float time;
        std::map<unsigned int, RigidBody *> bodies;
        std::vector<HardSphereInstance> bodyBoundingSpheres;
        std::vector<HardSphereInstance> bodyInternalSpheres;
        
        //Planes through which no rigid body may pass.
        std::vector<vec4> boundingPlanes;
        
        const size_t nrCollisionIterations;
        const float collisionEpsilon;
        const float collisionSphereMargin;
        SpatialSphereHasher boundingSphereHasher;
        SpatialSphereHasher internalSphereHasher;
};

}

}

