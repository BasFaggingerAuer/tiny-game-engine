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

//TODO: Replace these defines by class properties.

//Small margin above one to remove jitter.
#define RBEPS 0.001f
#define RBOPEPS 1.001f
#define RBMAXACC 10.0f
#define RBAABBSCALE 1.05f
#define RBAABBDT 0.5f

RigidBodySystem::RigidBodySystem(const int &a_nrSubSteps) :
    time(0.0f),
    totalEnergy(0.0f),
    totalLinearMomentum(0.0f),
    totalAngularMomentum(0.0f),
    bodies(),
    planeBodyIndices(),
    nrSubSteps(a_nrSubSteps)
{

}

RigidBodySystem::~RigidBodySystem()
{
    
}

float RigidBodySystem::getTime() const
{
    return time;
}

int RigidBodySystem::addInfinitePlaneBody(const vec4 &plane,
    const float &a_statFric, const float &a_dynFric, const float &a_rest, const float &a_soft)
{
    //Add an immovable infinite plane.
    planeBodyIndices.push_back(bodies.size());
    bodies.push_back({0.0f, vec3(0.0f),
        vec3(0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f), vec3(0.0f),
        false, RigidBodyGeometry::Plane,
        0.0f, 0.0f, 0, 0,
        plane/length(plane.xyz()),
        a_statFric, a_dynFric, a_rest, a_soft});

    //Infinite planes are not added to the AABB tree.
    
    std::cout << "Added plane " << bodies.back().plane << " " << bodies.back();

    return bodies.size() - 1;
}

int RigidBodySystem::addSpheresRigidBody(const float &totalMass, const std::vector<vec4> &a_spheres,
    const vec3 &a_x, const vec3 &v, const vec4 &a_q, const vec3 & a_w,
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

    vec3 x = a_x + centerOfMass;
    
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
    
    for (auto &s : spheres)
    {
        s = vec4(E.transposed()*s.xyz(), s.w);
    }
    
#ifndef NDEBUG
    //Verify that inertia tensor is indeed diagonal in these new coordinates.
    if (true)
    {
        mat3 inertiaCheck(0.0f);

        for (size_t i = 0; i < spheres.size(); ++i)
        {
            const vec4 s = spheres[i];
            
            inertiaCheck += (2.0f*massPerSphere[i]*s.w*s.w/5.0f)*mat3::identityMatrix() +
                            massPerSphere[i]*(length2(s.xyz())*mat3::identityMatrix() - mat3::outerProductMatrix(s.xyz(), s.xyz()));
        }

        assert((inertiaCheck - mat3::scaleMatrix(e)).getFrobeniusNorm() < EPS);
    }
#endif

    const vec4 q = quatmul(a_q, E.getRotation());
    //TODO: Check E or E.transposed().
    const vec3 w = E.transposed()*a_w;

    //Add rigid body to system.
    bodies.push_back({1.0f/totalMass, 1.0f/e, x, q, v, w, vec3(0.0f), vec3(0.0f),
        true, RigidBodyGeometry::Spheres,
        maxRadius, maxRadius, static_cast<int>(bodyInternalSpheres.size()), static_cast<int>(bodyInternalSpheres.size() + spheres.size()),
        vec4(0.0f),
        a_statFric, a_dynFric, a_rest, a_soft});
    bodyInternalSpheres.insert(bodyInternalSpheres.end(), spheres.begin(), spheres.end());

    //Add rigid body to the AABB tree.
    tree.insert(bodies.back().getAABB(RBAABBDT).scale(RBAABBSCALE), bodies.size() - 1);

    std::cout << "Added " << spheres.size() << " spheres " << bodies.back();

    return bodies.size() - 1;
}

void RigidBodySystem::addNonCollidingPair(const int &i1, const int &i2)
{
    if (i1 >= 0 && i2 >= 0 && i1 < static_cast<int>(bodies.size()) && i2 < static_cast<int>(bodies.size()))
    {
        nonCollidingBodies.insert(std::minmax(i1, i2));
    }
    else
    {
        std::cerr << "Invalid pair of non-colliding bodies specified!" << std::endl;
        assert(false);
    }
}

float applyAngularConstraint(const float lambda,
                             const float alpha,
                             RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &dq) noexcept
{
    vec3 n = normalize(dq);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return 0.0f;
    
    const mat3 invI1 = b1->getInvI();
    const mat3 invI2 = b2->getInvI();
    const float w1 = dot(n, invI1*n);
    const float w2 = dot(n, invI2*n);
    const float dlambda = -(length(dq) + alpha*lambda)/(w1 + w2 + alpha);
    
    n *= dlambda;

    if (b1->movable)
    {
        b1->q += 0.5f*quatmul(vec4(invI1*n, 0.0f), b1->q);
        b1->q = normalize(b1->q);
    }
    
    if (b2->movable)
    {
        b2->q += 0.5f*quatmul(vec4(invI2*n, 0.0f), b2->q);
        b2->q = normalize(b2->q);
    }

    return dlambda;
}

std::tuple<float, float, float> applyPositionConstraint(const float lambda,
                             const float alpha,
                             RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &p,
                             const vec3 &dx) noexcept
{
    vec3 n = normalize(dx);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return std::make_tuple(0.0f, 0.0f, 0.0f);

    const vec3 r1 = p - b1->x;
    const vec3 r2 = p - b2->x;
    const mat3 invI1 = b1->getInvI();
    const mat3 invI2 = b2->getInvI();
    const float w1 = b1->invM + dot(cross(r1, n), invI1*cross(r1, n));
    const float w2 = b2->invM + dot(cross(r2, n), invI2*cross(r2, n));
    const float dlambda = -(length(dx) + alpha*lambda)/(w1 + w2 + alpha);
    
    /*
    std::cout << "COLL PRE:" << std::endl
              << "p   : " << p << std::endl
              << "n   : " << n << std::endl
              << "dl  : " << dlambda << std::endl
              << "dx  : " << dx << std::endl
              << "b1.x: " << b1->x << std::endl
              << "b1.q: " << b1->q << std::endl
              << "b1.v: " << b1->v << std::endl
              << "b1.w: " << b1->w << std::endl
              << "b2.x: " << b2->x << std::endl
              << "b2.q: " << b2->q << std::endl
              << "b2.v: " << b2->v << std::endl
              << "b2.w: " << b2->w << std::endl;
    */
    
    n *= dlambda;

    if (b1->movable)
    {
        b1->x += b1->invM*n;
        b1->q += 0.5f*quatmul(vec4(invI1*cross(r1, n), 0.0f), b1->q);
        b1->q = normalize(b1->q);
    }
    
    if (b2->movable)
    {
        b2->x -= b2->invM*n;
        b2->q -= 0.5f*quatmul(vec4(invI2*cross(r2, n), 0.0f), b2->q);
        b2->q = normalize(b2->q);
    }
    
    /*
    std::cout << "COLL POST:" << std::endl
              << "n   : " << n << std::endl
              << "b1.x: " << b1->x << std::endl
              << "b1.q: " << b1->q << std::endl
              << "b2.x: " << b2->x << std::endl
              << "b2.q: " << b2->q << std::endl;
    */
    
    return std::make_tuple(dlambda, w1, w2);
}

void applyVelocityConstraint(RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &p,
                             const vec3 &dv) noexcept
{
    vec3 n = normalize(dv);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return;

    const vec3 r1 = p - b1->x;
    const vec3 r2 = p - b2->x;
    const float w1 = b1->invM + dot(cross(r1, n), b1->getInvI()*cross(r1, n));
    const float w2 = b2->invM + dot(cross(r2, n), b2->getInvI()*cross(r2, n));

    std::cout << "AVC N " << n << std::endl;
    std::cout << "AVC R1 PRE " << r1 << std::endl;
    std::cout << "AVC R2 PRE " << r2 << std::endl;
    std::cout << "AVC V1 PRE " << b1->v << std::endl;
    std::cout << "AVC V2 PRE " << b2->v << std::endl;
    
    n *= -length(dv)/(w1 + w2);

    b1->v += b1->invM*n;
    b1->w += b1->getInvI()*cross(r1, n);
    
    b2->v -= b2->invM*n;
    b2->w -= b2->getInvI()*cross(r2, n);
    
    std::cout << "AVC V1 POST " << b1->v << std::endl;
    std::cout << "AVC V2 POST " << b2->v << std::endl;
}

RigidBodyCollisionGeometry RigidBodySystem::getCollisionGeometry(std::vector<RigidBody> &bds, const RigidBodyCollision &c) const noexcept
{
    //Get pointers to bodies. (FIXME, rather inelegant.)
    RigidBody *b1 = &bds[c.b1Index];
    RigidBody *b2 = &bds[c.b2Index];

    vec3 p, n;
    float d; //Signed distance along n. Should be >= for no collision.
    
    if (b1->geometry == RigidBodyGeometry::Spheres && b2->geometry == RigidBodyGeometry::Spheres)
    {
        //Get potentially colliding internal spheres.
        vec4 is = bodyInternalSpheres[b1->firstInternalSphere + c.b1SphereIndex];
        const vec4 s1 = vec4(b1->x + mat3::rotationMatrix(b1->q)*is.xyz(), is.w);
        is = bodyInternalSpheres[b2->firstInternalSphere + c.b2SphereIndex];
        const vec4 s2 = vec4(b2->x + mat3::rotationMatrix(b2->q)*is.xyz(), is.w);

        n = normalize(s2.xyz() - s1.xyz());
        p = 0.5f*(s1.xyz() + s2.xyz()) + (0.5f*(s1.w - s2.w))*n;
        d = dot(s2.xyz() - s1.xyz(), n) - s1.w - s2.w;
    }
    else if (b1->geometry == RigidBodyGeometry::Plane && b2->geometry == RigidBodyGeometry::Spheres)
    {
        //Collide sphere with infinite plane.
        vec4 s2 = bodyInternalSpheres[b2->firstInternalSphere + c.b2SphereIndex];
        s2 = vec4(b2->x + mat3::rotationMatrix(b2->q)*s2.xyz(), s2.w);
        
        n = b1->plane.xyz();
        d = dot(s2.xyz(), n) - b1->plane.w - s2.w;
        p = s2.xyz() - (0.5f*(dot(s2.xyz(), n) + s2.w - b1->plane.w))*n;
    }
    else
    {
        std::cerr << "Unknown combination of body geometries!" << std::endl;
        assert(false);
    }

    return RigidBodyCollisionGeometry({b1, b2, d,
                                       p, n,
                                       b1->v + cross(b1->w, p - b1->x),
                                       b2->v + cross(b2->w, p - b2->x)});
}

aabb::aabb RigidBody::getAABB(const float &dt) const noexcept
{
    //Determine AABB with margin.
    const float r = radius + 0.5f*dt*dt*RBMAXACC + RBEPS;
    
    return aabb::aabb{x - vec3(r) + min(vec3(0.0f), dt*v), x + vec3(r) + max(vec3(0.0f), dt*v)};
}

void RigidBodySystem::update(const float &dt)
{
    //Per Detailed Rigid Body Simulation with Extended Position Based Dynamics by Matthias Muller et al., ACM SIGGRAPH, vol. 39, nr. 8, 2020.

    //Detect all collision pairs for the current state.

    //Check whether bounding boxes still contain objects in their current state and update AABB tree if not.
    for (size_t i = 0; i < bodies.size(); ++i)
    {
        const RigidBody &b = bodies[i];

        if (b.geometry == RigidBodyGeometry::Spheres)
        {
            if (!b.getAABB(dt).isSubsetOf(tree.getNodeBox(i)))
            {
                tree.erase(i);
                tree.insert(b.getAABB(RBAABBDT).scale(RBAABBSCALE), i);
            }
        }
    }

#ifndef NDEBUG
    tree.check();
#endif

    //Check which objects can potentially intersect.
    const auto intersectingObjects = tree.getOverlappingContents();
    
    //Find all collision points between potentially intersecting Spheres objects.
    //Use simple n^2 comparison (assume small number of internal spheres per object).
    std::vector<RigidBodyCollision> collisions;

    for (const auto &intersection : intersectingObjects)
    {
        //Are collisions allowed between these bodies?
        if (nonCollidingBodies.find(intersection) == nonCollidingBodies.end())
        {
            const RigidBody &b1 = bodies[intersection.first];
            const RigidBody &b2 = bodies[intersection.second];

            assert(b1.geometry == RigidBodyGeometry::Spheres);
            assert(b2.geometry == RigidBodyGeometry::Spheres);

            //TODO: Use that we could be testing 1 body vs. many other bodies to optimize.
            //TODO: Consider enforcing a fixed number of internal spheres per object to parallellize these checks effectively.
            const mat3 R1 = mat3::rotationMatrix(b1.q);
            const mat3 R2 = mat3::rotationMatrix(b2.q);

            for (auto iS1 = b1.firstInternalSphere; iS1 < b1.lastInternalSphere; ++iS1)
            {
                //Get potential collisions taking the object's velocity (linear and angular) into account.
                vec4 s1 = vec4(b1.x + R1*bodyInternalSpheres[iS1].xyz(), bodyInternalSpheres[iS1].w);

                s1.w += dt*(0.5f*dt*RBMAXACC + length(b1.v + cross(b1.w, s1.xyz() - b1.x))) + RBEPS;
                
                for (auto iS2 = b2.firstInternalSphere; iS2 < b2.lastInternalSphere; ++iS2)
                {
                    vec4 s2 = vec4(b2.x + R2*bodyInternalSpheres[iS2].xyz(), bodyInternalSpheres[iS2].w);

                    s2.w += dt*(0.5f*dt*RBMAXACC + length(b2.v + cross(b2.w, s2.xyz() - b2.x))) + RBEPS;
                    
                    if (length(s1.xyz() - s2.xyz()) <= s1.w + s2.w)
                    {
                        //If so, add a potential collision.
                        collisions.push_back(RigidBodyCollision({intersection.first, iS1 - b1.firstInternalSphere,
                                                                 intersection.second, iS2 - b2.firstInternalSphere}));
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

        for (int iB = 0; iB < static_cast<int>(bodies.size()); ++iB)
        {
            const RigidBody &b = bodies[iB];

            if (b.geometry == RigidBodyGeometry::Spheres)
            {
                //Can the body intersect with the plane?
                if (dot(b.x, p.xyz()) <= p.w + b.collisionRadius)
                {
                    const mat3 R = mat3::rotationMatrix(b.q);

                    for (auto iS = b.firstInternalSphere; iS < b.lastInternalSphere; ++iS)
                    {
                        //Get potential collisions taking the object's velocity (linear and angular) into account.
                        vec4 s = vec4(b.x + R*bodyInternalSpheres[iS].xyz(), bodyInternalSpheres[iS].w);

                        s.w += dt*(0.5f*dt*RBMAXACC + length(b.v + cross(b.w, s.xyz() - b.x))) + RBEPS;
                        
                        if (dot(s.xyz(), p.xyz()) <= p.w + s.w)
                        {
                            collisions.push_back(RigidBodyCollision({iP, 0, iB, iS - b.firstInternalSphere}));
                        }
                    }
                }
            }
        }
    }
    
    //Perform substeps.
    const float h = dt/static_cast<float>(nrSubSteps);

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
            if (cg.d < 0.0f)
            {
                const float softnessCoeff = 0.5f*(cg.b1->softness + cg.b2->softness)/(h*h);
                
                std::cout << "COLL " << i << ": " << c.b1Index << ", " << c.b1SphereIndex << " -- " << c.b2Index << ", " << c.b2SphereIndex << std::endl;
                std::cout << "COLL N " << cg.n << std::endl;
                std::cout << "COLL PRE X1 " << cg.b1->x << std::endl;
                std::cout << "COLL PRE X2 " << cg.b2->x << std::endl;
                std::cout << "COLL PRE V1 " << cg.b1->v << std::endl;
                std::cout << "COLL PRE V2 " << cg.b2->v << std::endl;
                
                //Apply position constraint to avoid object interpenetration.
                auto [l, w1, w2] = applyPositionConstraint(0.0f, softnessCoeff, cg.b1, cg.b2, cg.p, -cg.d*cg.n);

                std::cout << "COLL POST X1 " << cg.b1->x << std::endl;
                std::cout << "COLL POST X2 " << cg.b2->x << std::endl;
                
                lambdaCollisionN[i] = l;
                
                /*
                //Handle static friction.
                const float staticFrictionCoeff = std::sqrt(cg.b1->staticFriction*cg.b2->staticFriction);
                
                const RigidBody *preb1 = &preBodies[c.b1Index];
                const RigidBody *preb2 = &preBodies[c.b2Index];

                //Figure out how the current collision point has moved w.r.t. the previous iteration on both bodies.
                //N.B., we need to track a fixed point on the rigid body.
                const vec3 p1 = mat3::rotationMatrix(quatmul(preb1->q, quatconj(cg.b1->q)))*(cg.p - cg.b1->x) + preb1->x;
                const vec3 p2 = mat3::rotationMatrix(quatmul(preb2->q, quatconj(cg.b2->q)))*(cg.p - cg.b2->x) + preb2->x;

                vec3 dx = p2 - p1;

                dx -= dot(dx, cg.n)*cg.n;

                //Static friction, restrict tangential motion if F_tangential <= mu_static * F_normal.
                if (length(dx)/(w1 + w2) <= staticFrictionCoeff*std::abs(lambdaCollisionN[i]))
                {
                    applyPositionConstraint(0.0f, softnessCoeff, cg.b1, cg.b2, cg.p, dx);
                }
                
                std::cout << "COLL POST SF X1 " << cg.b1->x << std::endl;
                std::cout << "COLL POST SF X2 " << cg.b2->x << std::endl;
                */
            }
        }
        
        //Update velocities.
        for (size_t i = 0; i < bodies.size(); ++i)
        {
            if (bodies[i].movable)
            {
                bodies[i].v = (bodies[i].x - preBodies[i].x)/h;
    
                const vec4 dq = quatmul(bodies[i].q, quatconj(preBodies[i].q));
    
                bodies[i].w = (dq.w >= 0.0f ? 2.0f/h : -2.0f/h)*dq.xyz();
            }
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
                vt -= vn*cg.n;

                //Apply dynamic friction.
                /*
                const float dynamicFrictionCoeff = std::sqrt(cg.b1->dynamicFriction*cg.b2->dynamicFriction);

                applyVelocityConstraint(cg.b1, cg.b2, cg.p,
                                        std::min(dynamicFrictionCoeff*std::abs(lambdaCollisionN[i])/h, length(vt))*normalize(vt));
                */
                
                //Perform restitution in case of collisions that are not resting contacts.

                //Get pre-state normal velocity.
                const auto precg = getCollisionGeometry(preBodies, c);

                //Determine normal and tangential relative velocities.
                const float prevn = dot(precg.v1 - precg.v2, cg.n);

                const float restitutionCoeff = (std::abs(prevn) > h*RBMAXACC ? 
                                std::sqrt(cg.b1->restitution*cg.b2->restitution) :
                                0.0f);
                
                std::cout << "VN = " << vn << std::endl;
                std::cout << "N = " << cg.n << std::endl;
                std::cout << "V1_PRE = " << cg.b1->v << std::endl;
                std::cout << "V2_PRE = " << cg.b2->v << std::endl;
                std::cout << "PREVN = " << prevn << std::endl;
                std::cout << "COR = " << restitutionCoeff << std::endl;

                applyVelocityConstraint(cg.b1, cg.b2, cg.p,
                                        (vn + std::max(0.0f, restitutionCoeff*prevn))*cg.n);

                std::cout << "V1_POST = " << cg.b1->v << std::endl;
                std::cout << "V2_POST = " << cg.b2->v << std::endl;
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

    totalEnergy += potentialEnergy();
    
    //Increment global time.
    time += dt;
}

void RigidBodySystem::applyExternalForces()
{
    //To be implemented by the user (e.g., gravity).
    //b.f = vec3(0.0f, -9.81f/b.invM, 0.0f);
}

float RigidBodySystem::potentialEnergy() const
{
    //To be implemented by the user in accordance with applyExternalForces().
    return 0.0f;
}

