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
#include <set>

#include <tiny/rigid/rigidbody.h>

using namespace tiny;
using namespace tiny::rigid;

//Small margin above one to remove jitter.
#define RBEPS 0.001f
#define RBOPEPS 1.001f

SpatialSphereHasher::SpatialSphereHasher(const size_t &nrBuckets, const float &boxSize) :
    invBoxSize(1.0f/boxSize),
    buckets(nrBuckets, std::list<size_t>())
{

}

SpatialSphereHasher::~SpatialSphereHasher()
{

}

void SpatialSphereHasher::hashObjects(const std::vector<vec4> &objects)
{
    //Clear current buckets.
    for (std::vector<std::list<size_t>>::iterator i = buckets.begin(); i != buckets.end(); ++i)
    {
        i->clear();
    }

    //Sort spheres into buckets.
    size_t count = 0;
    
    for (std::vector<vec4>::const_iterator i = objects.begin(); i != objects.end(); ++i)
    {
        //Find range of boxes covered by this sphere.
        const ivec3 lo = to_int(floor(invBoxSize*(i->xyz() - i->w)));
        const ivec3 hi = to_int( ceil(invBoxSize*(i->xyz() + i->w)));
        
        for (int z = lo.z; z <= hi.z; ++z)
        {
            for (int y = lo.y; y <= hi.y; ++y)
            {
                for (int x = lo.x; x <= hi.x; ++x)
                {
                    //Chose three prime numbers for spatial hashing.
                    buckets[modnonneg(389*x + 1061*y + 599*z, static_cast<int>(buckets.size()))].push_back(count);
                }
            }
        }
        
        ++count;
    }
    
    assert(count == objects.size());
}

const std::vector<std::list<size_t>> *SpatialSphereHasher::getBuckets() const
{
    return &buckets;
}

RigidBodySystem::RigidBodySystem(const size_t &nrBuckets, const float &boundingSphereBucketSize, const float &internalSphereBucketSize, const float &a_collisionSphereMargin, const int &a_nrSubSteps) :
    time(0.0f),
    totalEnergy(0.0f),
    totalLinearMomentum(0.0f),
    totalAngularMomentum(0.0f),
    bodies(),
    planeBodyIndices(),
    collisionSphereMargin(a_collisionSphereMargin),
    nrSubSteps(a_nrSubSteps),
    boundingSphereHasher(nrBuckets, boundingSphereBucketSize),
    internalSphereHasher(nrBuckets, internalSphereBucketSize)
{

}

RigidBodySystem::~RigidBodySystem()
{
    
}

float RigidBodySystem::getTime() const
{
    return time;
}

void RigidBodySystem::addInfinitePlaneBody(const vec4 &plane,
    const float &a_statFric, const float &a_dynFric, const float &a_rest, const float &a_soft)
{
    //Add an immovable infinite plane.
    planeBodyIndices.push_back(bodies.size());
    bodies.push_back({0.0f, vec3(0.0f), vec3(0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f),
        false, RigidBodyGeometry::Plane,
        0.0f, 0, 0,
        plane/length(plane.xyz()),
        a_statFric, a_dynFric, a_rest, a_soft});
}

void RigidBodySystem::addSpheresRigidBody(const float &totalMass, const std::vector<vec4> &a_spheres,
    const vec3 &x, const vec3 &v, const vec4 &a_q, const vec3 & a_w,
    const float &a_statFric, const float &a_dynFric, const float &a_rest, const float &a_soft)
{
    //Add a rigid body to the system composed of spheres.

    //For now, distribute the mass uniformly over the volume of all spheres, assuming there are no overlaps.
    std::vector<vec4> spheres = a_spheres;
    std::vector<float> volumePerSphere(spheres.size());
    float totalVolume = 0.0f;
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        const float r = spheres[i].w;
        
        totalVolume += (volumePerSphere[i] = (4.0f*M_PI/3.0f)*r*r*r);
    }
    
    std::vector<float> massPerSphere(spheres.size());
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        massPerSphere[i] = totalMass*(volumePerSphere[i]/totalVolume);
    }
    
    //Ensure that the object's spheres are centered around the center of mass.
    vec3 centerOfMass = vec3(0.0f);
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        centerOfMass += massPerSphere[i]*spheres[i].xyz();
    }
    
    centerOfMass /= totalMass;
    
    for (auto &s : spheres)
    {
        s -= vec4(centerOfMass, 0.0f);
    }
    
    //Calculate bounding sphere containing all internal spheres.
    float maxRadius = 0.0f;
    
    for (const auto &s : spheres)
    {
        maxRadius = std::max(maxRadius, length(s.xyz()) + s.w);
    }
    
    //Calculate inertia tensor.
    mat3 inertia(0.0f);
    
    for (size_t i = 0; i < spheres.size(); ++i)
    {
        const vec4 s = spheres[i];
        
        inertia += (2.0f*massPerSphere[i]*s.w*s.w/5.0f)*mat3::identityMatrix() +
                   massPerSphere[i]*(length2(s.xyz())*mat3::identityMatrix() - mat3::outerProductMatrix(s.xyz(), s.xyz()));
    }

    //Rotate rigid body such that the intertia tensor is diagonal.
    const auto [e, E] = inertia.eigenDecompositionSym();
    const vec4 q = quatmul(a_q, quatconj(E.getRotation()));
    const vec3 w = mat3::rotationMatrix(quatconj(E.getRotation()))*a_w;

    //Add rigid body to system.
    bodies.push_back({1.0f/totalMass, 1.0f/e, x, q, v, w, vec3(0.0f), vec3(0.0f),
        true, RigidBodyGeometry::Spheres,
        maxRadius, bodyInternalSpheres.size(), bodyInternalSpheres.size() + spheres.size(),
        vec4(0.0f),
        a_statFric, a_dynFric, a_rest, a_soft});
    bodyInternalSpheres.insert(bodyInternalSpheres.end(), spheres.begin(), spheres.end());

    std::cout << "Added " << bodies.back();
}

float RigidBodySystem::addMarginToRadius(const float dt, const float r) const
{
    return (RBOPEPS + dt*collisionSphereMargin)*(r + RBEPS);
}

void RigidBodySystem::calculateInternalSpheres(const RigidBody &b, const float &dt)
{
    const mat3 R = mat3::rotationMatrix(b.q);

    for (size_t i = b.firstInternalSphere; i < b.lastInternalSphere; ++i)
    {
        const vec4 s = bodyInternalSpheres[i];

        collSpheres.push_back(vec4(b.x + R*s.xyz(), addMarginToRadius(dt, s.w)));
    }
}

float applyPositionConstraint(const float lambda,
                             const float alpha,
                             RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &p,
                             const vec3 &dx)
{
    vec3 n = normalize(dx);
    const vec3 r1 = p - b1->x;
    const vec3 r2 = p - b2->x;
    const float w1 = b1->invM + dot(cross(r1, n), b1->getInvI()*cross(r1, n));
    const float w2 = b2->invM + dot(cross(r2, n), b2->getInvI()*cross(r2, n));
    //Avoid numerical issues by enforcing a minimum separation after collisions.
    const float dlambda = -(std::max(RBEPS, RBOPEPS*length(dx)) + alpha*lambda)/(w1 + w2 + alpha);

    n *= dlambda;
    
    if (b1->movable)
    {
        b1->x += b1->invM*n;
        b1->q += 0.5f*quatmul(vec4(b1->getInvI()*cross(r1, n), 0.0f), b1->q);
        b1->q = normalize(b1->q);
    }
    
    if (b2->movable)
    {
        b2->x -= b2->invM*n;
        b2->q -= 0.5f*quatmul(vec4(b2->getInvI()*cross(r2, n), 0.0f), b2->q);
        b2->q = normalize(b2->q);
    }
    
    return dlambda;
}

void applyVelocityConstraint(RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &p,
                             const vec3 &dv)
{
    vec3 n = normalize(dv);
    const vec3 r1 = p - b1->x;
    const vec3 r2 = p - b2->x;
    const float w1 = b1->invM + dot(cross(r1, n), b1->getInvI()*cross(r1, n));
    const float w2 = b2->invM + dot(cross(r2, n), b2->getInvI()*cross(r2, n));
    
    n *= -length(dv)/(w1 + w2);

    b1->v += b1->invM*n;
    b1->w += b1->getInvI()*cross(r1, n);
    
    b2->v -= b2->invM*n;
    b2->w -= b2->getInvI()*cross(r2, n);
}

RigidBodyCollisionGeometry RigidBodySystem::getCollisionGeometry(std::vector<RigidBody> &bds, const RigidBodyCollision &c) const
{
    //Get pointers to bodies.
    RigidBody *b1 = &bds[c.b1Index];
    RigidBody *b2 = &bds[c.b2Index];
    RigidBodyCollisionGeometry cg;
    
    if (b1->geometry == RigidBodyGeometry::Spheres && b2->geometry == RigidBodyGeometry::Spheres)
    {
        //Get potentially colliding internal spheres.
        vec4 is = bodyInternalSpheres[b1->firstInternalSphere + c.b1SphereIndex];
        const vec4 s1 = vec4(b1->x + mat3::rotationMatrix(b1->q)*is.xyz(), is.w);
        is = bodyInternalSpheres[b2->firstInternalSphere + c.b2SphereIndex];
        const vec4 s2 = vec4(b2->x + mat3::rotationMatrix(b2->q)*is.xyz(), is.w);

        const vec3 n = normalize(s2.xyz() - s1.xyz());
        const vec3 p1 = s1.xyz() + s1.w*n;
        const vec3 p2 = s2.xyz() - s2.w*n;

        cg = RigidBodyCollisionGeometry({n,
                                         b1, p1, b1->v + cross(b1->w, p1 - b1->x),
                                         b2, p2, b2->v + cross(b2->w, p2 - b2->x),
                                         length(s1.xyz() - s2.xyz()) <= s1.w + s2.w});
    }
    else if (b1->geometry == RigidBodyGeometry::Plane && b2->geometry == RigidBodyGeometry::Spheres)
    {
        //Collide sphere with infinite plane.
        const vec4 p = b1->plane;
        vec4 s2 = bodyInternalSpheres[b2->firstInternalSphere + c.b2SphereIndex];
        s2 = vec4(b2->x + mat3::rotationMatrix(b2->q)*s2.xyz(), s2.w);
        const vec3 n = p.xyz();
        const float d = dot(s2.xyz(), n) - p.w;
        const vec3 p1 = s2.xyz() - d*n;
        const vec3 p2 = s2.xyz() - s2.w*n;

        cg = RigidBodyCollisionGeometry({n,
                                         b1, p1, vec3(0.0f),
                                         b2, p2, b2->v + cross(b2->w, p2 - b2->x),
                                         d <= s2.w});
    }
    else
    {
        std::cerr << "Unknown combination of body geometries!" << std::endl;
        assert(false);
    }

    return cg;
}

void RigidBodySystem::update(const float &dt)
{
    //Per Detailed Rigid Body Simulation with Extended Position Based Dynamics by Matthias Muller et al., ACM SIGGRAPH, vol. 39, nr. 8, 2020.

    //Detect all collision pairs for the current state.

    //This should not affect the capacity (so not cause spurious reallocations).
    collSpheres.clear();

    for (const auto &b : bodies)
    {
        //FIXME: Planes are also added, which is not necessary.
        collSpheres.push_back(vec4(b.x, addMarginToRadius(dt, b.radius)));
    }
    
    boundingSphereHasher.hashObjects(collSpheres);

    //Check which objects can potentially intersect.
    std::set<std::pair<size_t, size_t>> intersectingObjects;
    
    for (const auto &bucket : *boundingSphereHasher.getBuckets())
    {
        for (auto bucketB1 = bucket.cbegin(); bucketB1 != bucket.cend(); ++bucketB1)
        {
            for (auto bucketB2 = std::next(bucketB1); bucketB2 != bucket.cend(); ++bucketB2)
            {
                const vec4 s1 = collSpheres[*bucketB1];
                const vec4 s2 = collSpheres[*bucketB2];
                
                if ((length(s1.xyz() - s2.xyz()) <= addMarginToRadius(dt, s1.w + s2.w)) &&
                    (bodies[*bucketB1].geometry == RigidBodyGeometry::Spheres) &&
                    (bodies[*bucketB2].geometry == RigidBodyGeometry::Spheres))
                {
                    //Avoid storing both (A, B) and (B, A).
                    intersectingObjects.insert(std::minmax(*bucketB1, *bucketB2));
                }
            }
        }
    }

    //Find all collision points between potentially intersecting Spheres objects.
    std::vector<RigidBodyCollision> collisions;

    for (const auto &intersection : intersectingObjects)
    {
        const RigidBody &b1 = bodies[intersection.first];
        const RigidBody &b2 = bodies[intersection.second];
        const size_t nrB1Spheres = b1.lastInternalSphere - b1.firstInternalSphere;
        
        collSpheres.clear();
        calculateInternalSpheres(b1, dt);
        calculateInternalSpheres(b2, dt);
        internalSphereHasher.hashObjects(collSpheres);
        
        for (const auto &bucket : *internalSphereHasher.getBuckets())
        {
            //By construction, we know that spheres inside the buckets are sorted by object (i.e., internal spheres of b1 always precede internal spheres of b2 in each bucket).
            auto b2Start = bucket.cbegin();
            
            while (b2Start != bucket.cend() && *b2Start < nrB1Spheres) ++b2Start;
            
            for (auto bucketB1 = bucket.cbegin(); bucketB1 != b2Start; ++bucketB1)
            {
                for (auto bucketB2 = b2Start; bucketB2 != bucket.cend(); ++bucketB2)
                {
                    //Do the internal spheres intersect?
                    const vec4 s1 = collSpheres[*bucketB1];
                    const vec4 s2 = collSpheres[*bucketB2];

                    if (length(s1.xyz() - s2.xyz()) <= addMarginToRadius(dt, s1.w + s2.w))
                    {
                        //If so, add a potential collision.
                        collisions.push_back(RigidBodyCollision({intersection.first, *bucketB1,
                                                                 intersection.second, *bucketB2 - nrB1Spheres}));
                    }
                }
            }
        }
    }

    //Find all potential collisions between Spheres and Planes objects.
    for (auto iP : planeBodyIndices)
    {
        const vec4 p = bodies[iP].plane;

        assert(bodies[iP].geometry == RigidBodyGeometry::Plane);

        for (size_t iB = 0; iB < bodies.size(); ++iB)
        {
            const RigidBody &b = bodies[iB];

            if (b.geometry == RigidBodyGeometry::Spheres)
            {
                //Can the body intersect with the plane?
                if (dot(b.x, p.xyz()) <= p.w + addMarginToRadius(dt, b.radius))
                {
                    collSpheres.clear();
                    calculateInternalSpheres(b, dt);
                    
                    for (size_t iS = 0; iS < collSpheres.size(); ++iS)
                    {
                        const vec4 s = collSpheres[iS];

                        if (dot(s.xyz(), p.xyz()) <= p.w + addMarginToRadius(dt, s.w))
                        {
                            collisions.push_back(RigidBodyCollision({iP, 0, iB, iS}));
                        }
                    }
                }
            }
        }
    }
    
    //Perform substeps.
    const float h = dt/static_cast<float>(nrSubSteps);

    //Minimum velocity below which there is no restitution for collisions.
    const float minCollisionNormalVelocity = 0.1f; //2.0f*9.81f;
    
    //Zero force/torque accumulators.
    for (auto &b : bodies)
    {
        b.f = vec3(0.0f);
        b.t = vec3(0.0f);
    }

    //Apply forces.
    applyExternalForces();
    
    for (int iSubStep = 0; iSubStep < nrSubSteps; ++iSubStep)
    {
        //Store pre-update positions and velocities.
        preBodies = bodies;
        
        //Apply momenta.
        for (auto &b : bodies)
        {
            if (b.movable)
            {
                b.v += (h*b.invM)*b.f;
                b.x += h*b.v;

                b.w += h*(b.getInvI()*b.t);
                b.q += (0.5f*h)*quatmul(vec4(b.w, 0.0f), b.q);
                b.q = normalize(b.q);
            }
        }
        
        //Solve positions.
        
        //Collisions.
        std::vector<float> lambdaCollisionN(collisions.size(), 0.0f);
        
        for (size_t i = 0; i < collisions.size(); ++i)
        {
            const RigidBodyCollision c = collisions[i];
            auto cg = getCollisionGeometry(bodies, c);

            //Check for collision for the current body states.
            if (cg.isColliding)
            {
                const float d = dot(cg.n, cg.p1 - cg.p2);
                const float staticFrictionCoeff = std::sqrt(cg.b1->staticFriction*cg.b2->staticFriction);
                
                //Apply position constraint to avoid object interpenetration.
                lambdaCollisionN[i] = applyPositionConstraint(0.0f, 0.0f, cg.b1, cg.b2, 0.5f*(cg.p1 + cg.p2), d*cg.n);
                
                //Handle static friction.
                const float w1 = cg.b1->invM + dot(cross(cg.p1, cg.n), cg.b1->getInvI()*cross(cg.p1, cg.n));
                const float w2 = cg.b2->invM + dot(cross(cg.p2, cg.n), cg.b2->getInvI()*cross(cg.p2, cg.n));

                //Get pre-states of the rigid bodies.
                const auto precg = getCollisionGeometry(preBodies, c);
                
                //Get tangential motion.
                vec3 dx = (cg.p1 - precg.p1) - (cg.p2 - precg.p2);
                
                dx -= dot(dx, cg.n)*cg.n;

                //Static friction, restrict tangential motion if F_tangential <= mu_static * F_normal.
                if (length(dx)/(w1 + w2) <= staticFrictionCoeff*std::abs(lambdaCollisionN[i]))
                {
                    applyPositionConstraint(0.0f, 0.0f, cg.b1, cg.b2, 0.5f*(cg.p1 + cg.p2), dx);
                }
            }
        }
        
        //Update velocities.
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            bodies[i].v = (bodies[i].x - preBodies[i].x)/h;

            const vec4 dq = quatmul(bodies[i].q, quatconj(preBodies[i].q));

            bodies[i].w = (dq.w >= 0.0f ? 2.0f/h : -2.0f/h)*dq.xyz();
        }
        
        //Solve velocities.
        for (size_t i = 0; i < collisions.size(); ++i)
        {
            const RigidBodyCollision c = collisions[i];
            auto cg = getCollisionGeometry(bodies, c);

            //Did we have a collision?
            if (lambdaCollisionN[i] != 0.0f)
            {
                //Determine normal and tangential relative velocities.
                vec3 vt = cg.v1 - cg.v2;
                const float vn = dot(vt, cg.n);
                const float dynamicFrictionCoeff = std::sqrt(cg.b1->dynamicFriction*cg.b2->dynamicFriction);
                const float restitutionCoeff = std::sqrt(cg.b1->restitution*cg.b2->restitution);

                vt -= vn*cg.n;

                //Apply dynamic friction.
                applyVelocityConstraint(cg.b1, cg.b2, 0.5f*(cg.p1 + cg.p2),
                                        std::min(dynamicFrictionCoeff*std::abs(lambdaCollisionN[i])/h, length(vt))*normalize(vt));
                
                //Get pre-state normal velocity.
                const auto precg = getCollisionGeometry(preBodies, c);

                //Determine normal and tangential relative velocities.
                //FIXME: Use n or previous n?
                const float prevn = dot(precg.v1 - precg.v2, cg.n);

                //Perform restitution in case of collisions that are not resting contacts.
                if (std::abs(prevn) > h*minCollisionNormalVelocity)
                {
                    applyVelocityConstraint(cg.b1, cg.b2, 0.5f*(cg.p1 + cg.p2),
                                            (vn + std::max(0.0f, restitutionCoeff*prevn))*cg.n);
                }
            }
        }
    }

    //Calculate total energy and momenta.
    totalEnergy = 0.0f;
    totalLinearMomentum = vec3(0.0f);
    totalAngularMomentum = vec3(0.0f);
    
    for (const auto &b : bodies)
    {
        if (b.movable)
        {
            totalEnergy += (0.5f/b.invM)*length2(b.v) + 0.5f*dot(b.w, b.getI()*b.w);
            totalLinearMomentum += (1.0f/b.invM)*b.v;
            totalAngularMomentum += (1.0f/b.invM)*cross(b.x, b.v) + b.getI()*b.w;
        }
    }
    
    //Increment global time.
    time += dt;
}

void RigidBodySystem::applyExternalForces()
{
    //To be implemented by the user (e.g., gravity).
    //b.f = vec3(0.0f, -9.81f/b.invM, 0.0f);
}

