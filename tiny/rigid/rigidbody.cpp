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
#include <climits>
#include <exception>
#include <algorithm>

#include <tiny/rigid/rigidbody.h>

using namespace tiny;
using namespace tiny::rigid;

RigidBody::RigidBody(const float &mass, const std::vector<HardSphereInstance> &a_spheres, const vec3 &a_position, const vec4 &a_orientation, const vec3 &a_linearMomentum, const vec3 &a_angularMomentum) :
    spheres(a_spheres),
    inverseMass(1.0f/mass),
    position(a_position),
    orientation(a_orientation),
    linearMomentum(a_linearMomentum),
    angularMomentum(a_angularMomentum)
{
    calculateSphereParameters();
}

RigidBody::RigidBody(const RigidBody &a) :
    spheres(a.spheres),
    boundingSphere(a.boundingSphere),
    inverseMass(a.inverseMass),
    inverseInertia(a.inverseInertia),
    position(a.position),
    orientation(a.orientation),
    linearMomentum(a.linearMomentum),
    angularMomentum(a.angularMomentum)
{
    calculateSphereParameters();
}

RigidBody::~RigidBody()
{

}

float RigidBody::getEnergy() const
{
    //Calculate energy of this rigid body.
    const mat3 invI = mat3(orientation)*inverseInertia*mat3(quatconj(orientation));

    return 0.5f*inverseMass*length2(linearMomentum) + 0.5f*dot(angularMomentum, invI*angularMomentum);
}

void RigidBody::calculateSphereParameters()
{
    //For now, distribute the mass uniformly over the volume of all spheres, assuming there are no overlaps.
    std::vector<float> volumePerSphere(spheres.size(), 0.0f);
    float totalVolume = 0.0f;
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        const float r = spheres[i].posAndRadius.w;
        
        totalVolume += (volumePerSphere[i] = (4.0f*M_PI/3.0f)*r*r*r);
    }
    
    const float totalMass = 1.0f/inverseMass;
    std::vector<float> massPerSphere(spheres.size(), 0.0f);
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        massPerSphere[i] = totalMass*(volumePerSphere[i]/totalVolume);
    }
    
    //Ensure that the object's spheres are centered around the center of mass.
    vec3 centerOfMass = vec3(0.0f, 0.0f, 0.0f);
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        centerOfMass += massPerSphere[i]*spheres[i].posAndRadius.xyz();
    }
    
    centerOfMass /= totalMass;
    
    for (std::vector<HardSphereInstance>::iterator i = spheres.begin(); i != spheres.end(); ++i)
    {
        i->posAndRadius -= vec4(centerOfMass, 0.0f);
    }
    
    //Calculate bounding sphere containing all internal spheres.
    float maxRadius = 0.0f;
    
    for (std::vector<HardSphereInstance>::const_iterator i = spheres.begin(); i != spheres.end(); ++i)
    {
        maxRadius = std::max(maxRadius, length(i->posAndRadius.xyz()) + i->posAndRadius.w);
    }
    
    boundingSphere = vec4(0.0f, 0.0f, 0.0f, maxRadius);
    
    //Calculate inverse inertia tensor.
    mat3 inertia(0.0f);
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        const vec4 s = spheres[i].posAndRadius;
        
        inertia += (2.0f*massPerSphere[i]*s.w*s.w/5.0f)*mat3::identityMatrix() +
                   massPerSphere[i]*(length2(s.xyz())*mat3::identityMatrix() - mat3::outerProductMatrix(s.xyz(), s.xyz()));
    }
    
    inverseInertia = inertia.inverted();
}

void RigidBody::collide(RigidBody &a, RigidBody &b, const vec3 &z, const float &elasticity)
{
    //Perform collision between two rigid bodies, conserving linear momentum, angular momentum and energy if the elasticity == 1.0.
    //FIXME: This is incorrect!
    const vec3 ab = b.position - a.position;
    const vec3 az = z - a.position;
    const vec3 bz = z - b.position;
    const vec3 axz = cross(az, ab);
    const vec3 bxz = cross(bz, ab);
    const vec3 p = (-(1.0f + elasticity)*
                     (  a.inverseMass*dot(a.linearMomentum, ab) + dot(axz, a.inverseInertia*a.angularMomentum)
                      - b.inverseMass*dot(b.linearMomentum, ab) - dot(bxz, b.inverseInertia*b.angularMomentum))/
                     (  a.inverseMass*length2(ab) + dot(axz, a.inverseInertia*axz)
                      + b.inverseMass*length2(ab) + dot(bxz, b.inverseInertia*bxz)))*
                   ab;

    a.linearMomentum += p;
    a.angularMomentum += cross(az, p);
    
    b.linearMomentum -= p;
    b.angularMomentum -= cross(bz, p);
}

SpatialSphereHasher::SpatialSphereHasher(const size_t &nrBuckets, const float &boxSize) :
    invBoxSize(1.0f/boxSize),
    buckets(nrBuckets, std::list<size_t>())
{

}

SpatialSphereHasher::~SpatialSphereHasher()
{

}

void SpatialSphereHasher::hashObjects(const std::vector<HardSphereInstance> &objects)
{
    //Clear current buckets.
    for (std::vector<std::list<size_t>>::iterator i = buckets.begin(); i != buckets.end(); ++i)
    {
        i->clear();
    }

    //Sort spheres into buckets.
    size_t count = 0;
    
    for (std::vector<HardSphereInstance>::const_iterator i = objects.begin(); i != objects.end(); ++i)
    {
        //Find range of boxes covered by this sphere.
        const ivec4 lo = vfloor(invBoxSize*(i->posAndRadius - i->posAndRadius.w));
        const ivec4 hi =  vceil(invBoxSize*(i->posAndRadius + i->posAndRadius.w));
        
        for (int z = lo.z; z <= hi.z; ++z)
        {
            for (int y = lo.y; y <= hi.y; ++y)
            {
                for (int x = lo.x; x <= hi.x; ++x)
                {
                    //Chose three prime numbers for spatial hashing.
                    buckets[modnonneg(389*x + 1061*y + 599*z, static_cast<int>(buckets.size()))].push_back(count++);
                }
            }
        }
    }
}

const std::vector<std::list<size_t>> &SpatialSphereHasher::getBuckets() const
{
    return buckets;
}

RigidBodySystem::RigidBodySystem(const size_t &a_nrCollisionIterations, const size_t &nrBuckets, const float &boundingSphereBucketSize, const float &internalSphereBucketSize) :
    time(0.0f),
    bodies(),
    boundingPlanes(),
    nrCollisionIterations(a_nrCollisionIterations),
    boundingSphereHasher(nrBuckets, boundingSphereBucketSize),
    internalSphereHasher(nrBuckets, internalSphereBucketSize)
{

}

RigidBodySystem::~RigidBodySystem()
{
    //The user should free the rigid bodies.
}

void RigidBodySystem::addRigidBody(const unsigned int &index, RigidBody *body)
{
    //Verify that this index is not yet in use.
    std::map<unsigned int, RigidBody *>::iterator j = bodies.find(index);
    
    if (j != bodies.end())
    {
        std::cerr << "A rigid body with index " << index << " has already been added!" << std::endl;
        throw std::exception();
    }
    
    bodies.insert(std::make_pair(index, body));
}

bool RigidBodySystem::freeRigidBody(const unsigned int &index)
{
    //Verify that this index is in use.
    std::map<unsigned int, RigidBody *>::iterator j = bodies.find(index);
    
    if (j == bodies.end())
    {
        std::cerr << "Unable to find rigid body with index " << index << "!" << std::endl;
        return false;
    }
    
    bodies.erase(j);
    
    return true;
}

void RigidBodySystem::integratePositionsAndCalculateBodySpheres(const float &dt)
{
    //This could be separated and parallelized for better performance.
    
    //This should not affect the capacity (so not cause spurious reallocations).
    bodyBoundingSpheres.clear();
    bodyInternalSpheres.clear();
    
    for (std::map<unsigned int, RigidBody *>::const_iterator i = bodies.begin(); i != bodies.end(); ++i)
    {
        //Integrate position and orientation.
        const RigidBody *b = i->second;
        //x + dt*v, v = Minv*P
        const vec3 x = b->position + (dt*b->inverseMass)*b->linearMomentum;
        //Iinv = R*Iinv0*Rinv
        const mat3 invI = mat3::rotationMatrix(b->orientation)*b->inverseInertia*mat3::rotationMatrix(quatconj(b->orientation));
        //q + dt*0.5*(omega, 0)*q, omega = Iinv*L
        const mat3 R = mat3::rotationMatrix(normalize(b->orientation + (dt*0.5f)*quatmul(vec4(invI*b->angularMomentum, 0.0f), b->orientation)));
        
        bodyBoundingSpheres.push_back(vec4(x, b->boundingSphere.posAndRadius.w));
        
        for (std::vector<HardSphereInstance>::const_iterator j = b->spheres.begin(); j != b->spheres.end(); ++j)
        {
            bodyInternalSpheres.push_back(vec4(x + R*j->posAndRadius.xyz(), j->posAndRadius.w));
        }
    }
}

void RigidBodySystem::update(const float &dt)
{
    //Use the semi-implicit Euler method as symplectic integrator.
    
    //Positions.
    bool noCollisions = false;
    
    for (size_t i = 0; i < nrCollisionIterations && !noCollisions; ++i)
    {
        //Integrate positions.
        integratePositionsAndCalculateBodySpheres(dt);
        
        //Check for collisions.
        noCollisions = true;
        
        boundingSphereHasher.hashObjects(bodyBoundingSpheres);
        
        //Check which objects can potentially intersect.
        const std::vector<std::list<size_t>> &buckets = boundingSphereHasher.getBuckets();
        std::list<std::pair<size_t, size_t>> intersectingObjects;
        
        for (std::vector<std::list<size_t>>::const_iterator j = buckets.begin(); j != buckets.end(); ++j)
        {
            for (std::list<size_t>::const_iterator k = j->begin(); k != j->end(); ++k)
            {
                for (std::list<size_t>::const_iterator l = std::next(k); l != j->end(); ++l)
                {
                    if (length(bodyBoundingSpheres[*k].posAndRadius.xyz() - bodyBoundingSpheres[*l].posAndRadius.xyz()) <=
                               bodyBoundingSpheres[*k].posAndRadius.w + bodyBoundingSpheres[*l].posAndRadius.w)
                    {
                        intersectingObjects.push_back(std::make_pair(*k, *l));
                    }
                }
            }
        }
    }
    
    //Query forces.
    
    //Velocities.
    
    //Increment global time.
    time += dt;
}

void RigidBodySystem::applyExternalForces()
{
    //To be implemented by the user (e.g., gravity).
}

