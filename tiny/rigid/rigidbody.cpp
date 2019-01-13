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
#include <cassert>
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

const std::vector<std::list<size_t>> *SpatialSphereHasher::getBuckets() const
{
    return &buckets;
}

RigidBodySystem::RigidBodySystem(const size_t &a_nrCollisionIterations, const float &a_collisionEpsilon, const size_t &nrBuckets, const float &boundingSphereBucketSize, const float &internalSphereBucketSize) :
    time(0.0f),
    bodies(),
    boundingPlanes(),
    nrCollisionIterations(a_nrCollisionIterations),
    collisionEpsilon(a_collisionEpsilon),
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

void RigidBodySystem::integratePositionsAndCalculateBoundingSpheres(const float &dt)
{
    //This could be separated and parallelized for better performance.
    
    //This should not affect the capacity (so not cause spurious reallocations).
    bodyBoundingSpheres.clear();
    
    for (std::map<unsigned int, RigidBody *>::const_iterator i = bodies.begin(); i != bodies.end(); ++i)
    {
        //Integrate position and orientation.
        const RigidBody *b = i->second;
        //x + dt*v, v = Minv*P
        const vec3 x = b->position + (dt*b->inverseMass)*b->linearMomentum;
        
        bodyBoundingSpheres.push_back(vec4(x, b->boundingSphere.posAndRadius.w));
    }
}

void RigidBodySystem::integratePositionsAndCalculateInternalSpheres(const RigidBody *b, const float &dt)
{
    //Integrate position and orientation.
    //x + dt*v, v = Minv*P
    const vec3 x = b->position + (dt*b->inverseMass)*b->linearMomentum;
    //Iinv = R*Iinv0*Rinv
    const mat3 invI = mat3::rotationMatrix(b->orientation)*b->inverseInertia*mat3::rotationMatrix(quatconj(b->orientation));
    //q + dt*0.5*(omega, 0)*q, omega = Iinv*L
    const mat3 R = mat3::rotationMatrix(normalize(b->orientation + (dt*0.5f)*quatmul(vec4(invI*b->angularMomentum, 0.0f), b->orientation)));
    
    for (std::vector<HardSphereInstance>::const_iterator j = b->spheres.begin(); j != b->spheres.end(); ++j)
    {
        bodyInternalSpheres.push_back(vec4(x + R*j->posAndRadius.xyz(), j->posAndRadius.w));
    }
}

void RigidBodySystem::update(const float &dt)
{
    //Use the semi-implicit Euler method as symplectic integrator.
    
    //Update positions.
    bool noCollisions = false;
    float elasticity = 1.0f;
    
    for (size_t iCollision = 0; iCollision < nrCollisionIterations && !noCollisions; ++iCollision)
    {
        //Remove all energy for the final collision round to prevent object intersection.
        elasticity *= (iCollision < nrCollisionIterations - 1 ? 1.0f : 0.0f);
        
        //Integrate positions.
        integratePositionsAndCalculateBoundingSpheres(dt);
        
        //Check for collisions.
        noCollisions = true;
        
        boundingSphereHasher.hashObjects(bodyBoundingSpheres);
        
        //Check which objects can potentially intersect.
        const std::vector<std::list<size_t>> *buckets = boundingSphereHasher.getBuckets();
        std::list<std::pair<size_t, size_t>> intersectingObjects;
        
        for (std::vector<std::list<size_t>>::const_iterator bucket = buckets->begin(); bucket != buckets->end(); ++bucket)
        {
            for (std::list<size_t>::const_iterator bucketB1 = bucket->begin(); bucketB1 != bucket->end(); ++bucketB1)
            {
                for (std::list<size_t>::const_iterator bucketB2 = std::next(bucketB1); bucketB2 != bucket->end(); ++bucketB2)
                {
                    if (length(bodyBoundingSpheres[*bucketB1].posAndRadius.xyz() - bodyBoundingSpheres[*bucketB2].posAndRadius.xyz()) <=
                               bodyBoundingSpheres[*bucketB1].posAndRadius.w + bodyBoundingSpheres[*bucketB2].posAndRadius.w)
                    {
                        intersectingObjects.push_back(std::make_pair(*bucketB1, *bucketB2));
                    }
                }
            }
        }
        
        //Intersect internal spheres of these potentially intersecting objects and resolve collisions.
        for (std::list<std::pair<size_t, size_t>>::const_iterator intersection = intersectingObjects.begin(); intersection != intersectingObjects.end(); ++intersection)
        {
            RigidBody *b1 = bodies[intersection->first];
            RigidBody *b2 = bodies[intersection->second];
            
            bodyInternalSpheres.clear();
            integratePositionsAndCalculateInternalSpheres(b1, dt);
            integratePositionsAndCalculateInternalSpheres(b2, dt);
            internalSphereHasher.hashObjects(bodyInternalSpheres);
            buckets = internalSphereHasher.getBuckets();
            
            for (std::vector<std::list<size_t>>::const_iterator bucket = buckets->begin(); bucket != buckets->end(); ++bucket)
            {
                //By construction, we know that spheres inside the buckets are sorted by object (i.e., internal spheres of b1 always precede internal spheres of b2 in each bucket).
                std::list<size_t>::const_iterator b2Start = bucket->begin();
                
                while (b2Start != bucket->end() && *b2Start < b1->spheres.size()) ++b2Start;
                
                for (std::list<size_t>::const_iterator bucketB1 = bucket->begin(); bucketB1 != b2Start; ++bucketB1)
                {
                    for (std::list<size_t>::const_iterator bucketB2 = b2Start; bucketB2 != bucket->end(); ++bucketB2)
                    {
                        //Do the internal spheres intersect?
                        if (length(bodyBoundingSpheres[*bucketB1].posAndRadius.xyz() - bodyBoundingSpheres[*bucketB2].posAndRadius.xyz()) <=
                               bodyBoundingSpheres[*bucketB1].posAndRadius.w + bodyBoundingSpheres[*bucketB2].posAndRadius.w)
                        {
                            const vec4 s1 = bodyBoundingSpheres[*bucketB1].posAndRadius;
                            const vec4 s2 = bodyBoundingSpheres[*bucketB2].posAndRadius;
                            const vec3 z = (1.0f/(s1.w + s2.w))*(s2.w*s1.xyz() + s1.w*s2.xyz());
                            const vec3 n = normalize(s1.xyz() - s2.xyz());
                            const mat3 invI1 = mat3::rotationMatrix(b1->orientation)*b1->inverseInertia*mat3::rotationMatrix(quatconj(b1->orientation));
                            const mat3 invI2 = mat3::rotationMatrix(b2->orientation)*b2->inverseInertia*mat3::rotationMatrix(quatconj(b2->orientation));
                            
                            //Calculate relative velocity with small bias to prevent single-precision fluctuations from causing spurious collisions.
                            const float relVelocity = dot(n, (b1->inverseMass*b1->linearMomentum + cross(invI1*b1->angularMomentum, z - b1->position)) - 
                                                             (b2->inverseMass*b2->linearMomentum + cross(invI2*b2->angularMomentum, z - b2->position)));
                            
                            if (relVelocity < -collisionEpsilon)
                            {
                                //We have a collision --> update bodies.
                                const vec3 z1 = cross(z - b1->position, n);
                                const vec3 z2 = cross(z - b2->position, n);
                                const float alpha = -(1.0f + elasticity)*
                                                   (  b1->inverseMass*dot(b1->linearMomentum, n) + dot(z1, invI1*b1->angularMomentum)
                                                    - b2->inverseMass*dot(b2->linearMomentum, n) - dot(z2, invI2*b2->angularMomentum))/
                                                   (  b1->inverseMass + dot(z1, invI1*z1)
                                                    + b2->inverseMass + dot(z2, invI2*z2));
                                
                                b1->linearMomentum += alpha*n;
                                b1->angularMomentum += alpha*z1;
                                
                                b2->linearMomentum -= alpha*n;
                                b2->angularMomentum -= alpha*z2;
                                
                                noCollisions = false;
                            }
                            else if (relVelocity <= collisionEpsilon)
                            {
                                //TODO: Store this as a resting contact.
                            }
                            //else the objects are separating, which is fine.
                        }
                    }
                }
            }
        }
    }

#ifndef NDEBUG
    //Check that all colliding objects are indeed separating.
    if (true)
    {
        boundingSphereHasher.hashObjects(bodyBoundingSpheres);
        
        //Check which objects can potentially intersect.
        const std::vector<std::list<size_t>> *buckets = boundingSphereHasher.getBuckets();
        std::list<std::pair<size_t, size_t>> intersectingObjects;
        
        for (std::vector<std::list<size_t>>::const_iterator bucket = buckets->begin(); bucket != buckets->end(); ++bucket)
        {
            for (std::list<size_t>::const_iterator bucketB1 = bucket->begin(); bucketB1 != bucket->end(); ++bucketB1)
            {
                for (std::list<size_t>::const_iterator bucketB2 = std::next(bucketB1); bucketB2 != bucket->end(); ++bucketB2)
                {
                    if (length(bodyBoundingSpheres[*bucketB1].posAndRadius.xyz() - bodyBoundingSpheres[*bucketB2].posAndRadius.xyz()) <=
                               bodyBoundingSpheres[*bucketB1].posAndRadius.w + bodyBoundingSpheres[*bucketB2].posAndRadius.w)
                    {
                        intersectingObjects.push_back(std::make_pair(*bucketB1, *bucketB2));
                    }
                }
            }
        }
        
        //Intersect internal spheres of these potentially intersecting objects and resolve collisions.
        for (std::list<std::pair<size_t, size_t>>::const_iterator intersection = intersectingObjects.begin(); intersection != intersectingObjects.end(); ++intersection)
        {
            RigidBody *b1 = bodies[intersection->first];
            RigidBody *b2 = bodies[intersection->second];
            
            bodyInternalSpheres.clear();
            integratePositionsAndCalculateInternalSpheres(b1, dt);
            integratePositionsAndCalculateInternalSpheres(b2, dt);
            internalSphereHasher.hashObjects(bodyInternalSpheres);
            buckets = internalSphereHasher.getBuckets();
            
            for (std::vector<std::list<size_t>>::const_iterator bucket = buckets->begin(); bucket != buckets->end(); ++bucket)
            {
                //By construction, we know that spheres inside the buckets are sorted by object (i.e., internal spheres of b1 always precede internal spheres of b2 in each bucket).
                std::list<size_t>::const_iterator b2Start = bucket->begin();
                
                while (b2Start != bucket->end() && *b2Start < b1->spheres.size()) ++b2Start;
                
                for (std::list<size_t>::const_iterator bucketB1 = bucket->begin(); bucketB1 != b2Start; ++bucketB1)
                {
                    for (std::list<size_t>::const_iterator bucketB2 = b2Start; bucketB2 != bucket->end(); ++bucketB2)
                    {
                        //Do the internal spheres intersect?
                        if (length(bodyBoundingSpheres[*bucketB1].posAndRadius.xyz() - bodyBoundingSpheres[*bucketB2].posAndRadius.xyz()) <=
                               bodyBoundingSpheres[*bucketB1].posAndRadius.w + bodyBoundingSpheres[*bucketB2].posAndRadius.w)
                        {
                            const vec4 s1 = bodyBoundingSpheres[*bucketB1].posAndRadius;
                            const vec4 s2 = bodyBoundingSpheres[*bucketB2].posAndRadius;
                            const vec3 z = (1.0f/(s1.w + s2.w))*(s2.w*s1.xyz() + s1.w*s2.xyz());
                            const vec3 n = normalize(s1.xyz() - s2.xyz());
                            const mat3 invI1 = mat3::rotationMatrix(b1->orientation)*b1->inverseInertia*mat3::rotationMatrix(quatconj(b1->orientation));
                            const mat3 invI2 = mat3::rotationMatrix(b2->orientation)*b2->inverseInertia*mat3::rotationMatrix(quatconj(b2->orientation));
                            
                            //Calculate relative velocity with small bias to prevent single-precision fluctuations from causing spurious collisions.
                            const float relVelocity = dot(n, (b1->inverseMass*b1->linearMomentum + cross(invI1*b1->angularMomentum, z - b1->position)) - 
                                                             (b2->inverseMass*b2->linearMomentum + cross(invI2*b2->angularMomentum, z - b2->position)));
                            
                            if (relVelocity < -collisionEpsilon)
                            {
                                //We should no longer have interpenetrating objects moving further into each other.
                                assert(false);
                            }
                        }
                    }
                }
            }
        }
    }
#endif
    
    //Query forces.
    applyExternalForces(dt);
    
    //Update velocities.
    //TODO: Add normal force/resting contact calculation.
    
    //Increment global time.
    time += dt;
}

void RigidBodySystem::applyExternalForces(const float &)
{
    //To be implemented by the user (e.g., gravity).
    //b->linearMomentum += dt*vec3(0.0f, -9.81f/b->inverseMass, 0.0f);
}

