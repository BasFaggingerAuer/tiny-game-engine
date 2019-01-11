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
        
        static void collide(RigidBody &, RigidBody &, const vec3 &, const float &);

        //Rigid body collision detection is performed by decomposing the rigid body into hard spheres and performing only sphere-sphere collisions. This permits us to not consider all vertex/edge/plane collision cases at the cost of increased computation time.
        std::vector<HardSphereInstance> spheres;
        HardSphereInstance boundingSphere;
        
        float inverseMass;
        mat3 inverseInertia;

        vec3 position;
        vec4 orientation;
        vec3 linearMomentum;
        vec3 angularMomentum;
        
};

class SpatialSphereHasher
{
    public:
        SpatialSphereHasher(const size_t &, const float &);
        ~SpatialSphereHasher();
        
        void hashObjects(const std::vector<HardSphereInstance> &);
        const std::vector<std::list<size_t>> &getBuckets() const;
        
    private:
        const float invBoxSize;
        std::vector<std::list<size_t>> buckets;
};

class RigidBodySystem
{
    public:
        RigidBodySystem(const size_t & = 4);
        virtual ~RigidBodySystem();
        
        void addRigidBody(const unsigned int &, RigidBody *);
        bool freeRigidBody(const unsigned int &);
        
        void update(const float &);
        
    protected:
        void integratePositionsAndCalculateBodySpheres(const float &);
        virtual void applyExternalForces();
    
        float time;
        std::map<unsigned int, RigidBody *> bodies;
        std::vector<HardSphereInstance> bodyBoundingSpheres;
        std::vector<HardSphereInstance> bodyInternalSpheres;
        
        //Planes through which no rigid body may pass.
        std::vector<vec4> boundingPlanes;
        
        const size_t nrCollisionIterations;
};

}

}

