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

#include <tiny/rigid/triangle.h>
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
    constraints(),
    nrSubSteps(a_nrSubSteps)
{
    //Add rigid body 0 to contain triangles.
    //FIXME: This can be done more elegantly.
    //Use ~gravel material coefficients.
    addImmovableSpheresRigidBody({vec4(0.0f, 0.0f, 0.0f, 1.0f)}, vec3(0.0f), 1.0f, 0.6f, 0.4f, 1.0e-5f);
    bodies[0].canCollide = false;
}

RigidBodySystem::~RigidBodySystem()
{
    
}

float RigidBodySystem::getTime() const
{
    return time;
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

        //FIXME: Can we achieve 1e-6 single precision for high-mass objects?
        assert((inertiaCheck - mat3::scaleMatrix(e)).getFrobeniusNorm() < 1.0e-4);
    }
#endif

    const vec4 q = quatmul(a_q, E.getRotation());
    //TODO: Check E or E.transposed().
    const vec3 w = E.transposed()*a_w;

    //Add rigid body to system.
    bodies.push_back({1.0f/totalMass, 1.0f/e, x, q, v, w, vec3(0.0f), vec3(0.0f),
        true, true,
        maxRadius, maxRadius, static_cast<int>(bodyInternalSpheres.size()), static_cast<int>(bodyInternalSpheres.size() + spheres.size()),
        a_statFric, a_dynFric, a_rest, a_soft});
    bodyInternalSpheres.insert(bodyInternalSpheres.end(), spheres.begin(), spheres.end());

    //Add rigid body to the AABB tree.
    tree.insert(bodies.back().getAABB(RBAABBDT).scale(RBAABBSCALE), bodies.size() - 1);

    std::cout << "Added " << spheres.size() << " spheres, index " << bodies.size() - 1 << ", " << bodies.back();

    return bodies.size() - 1;
}

int RigidBodySystem::addImmovableSpheresRigidBody(const std::vector<vec4> &a_spheres, const vec3 &a_x,
    const float &a_statFric, const float &a_dynFric, const float &a_rest, const float &a_soft)
{
    //Add a fixed rigid body to the system composed of spheres.
    const int index = addSpheresRigidBody(1.0f, a_spheres, a_x, vec3(0.0f, 0.0f, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), a_statFric, a_dynFric, a_rest, a_soft);

    std::cout << "    immovable" << std::endl;

    bodies[index].movable = false;
    bodies[index].invM = 0.0f;
    bodies[index].invI = vec3(0.0f, 0.0f, 0.0f);

    return index;
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

int RigidBodySystem::addPositionConstraint(const int &i1, const vec3 &r1, const int &i2, const vec3 &r2, const float &d, const float &alpha)
{
    std::cout << "Adding position constraint between " << i1 << " " << r1 << " and " << i2 << " " << r2 << "." << std::endl;

    if (i1 < 0 || i2 < 0 || i1 >= static_cast<int>(bodies.size()) || i2 >= static_cast<int>(bodies.size()) || i1 == i2)
    {
        std::cerr << "Invalid pair of bodies for applying a position constraint!" << std::endl;
        assert(false);
        return 0;
    }
    
    constraints.push_back(Constraint{{i1, 0, r1}, {i2, 0, r2}, Constraint::Position, vec3(0.0f), d, alpha, 0.0f, false});

    return constraints.size() - 1;
}

int RigidBodySystem::addPositionLineConstraint(const int &i1, const vec3 &r1, const vec3 &n, const int &i2, const vec3 &r2, const float &d, const float &alpha)
{
    std::cout << "Adding position line constraint between " << i1 << " " << r1 << " and " << i2 << " " << r2 << " along direction " << n << "." << std::endl;

    if (i1 < 0 || i2 < 0 || i1 >= static_cast<int>(bodies.size()) || i2 >= static_cast<int>(bodies.size()) || i1 == i2)
    {
        std::cerr << "Invalid pair of bodies for applying a position line constraint!" << std::endl;
        assert(false);
        return 0;
    }
    
    constraints.push_back(Constraint{{i1, 0, r1}, {i2, 0, r2}, Constraint::PositionOnLine, n, d, alpha, 0.0f, false});
    
    return constraints.size() - 1;
}

int RigidBodySystem::addAngularConstraint(const int &i1, const vec3 &r1, const int &i2, const vec3 &r2, const float &d, const float &alpha)
{
    std::cout << "Adding angular constraint between " << i1 << " " << r1 << " and " << i2 << " " << r2 << "." << std::endl;

    if (i1 < 0 || i2 < 0 || i1 >= static_cast<int>(bodies.size()) || i2 >= static_cast<int>(bodies.size()) || i1 == i2)
    {
        std::cerr << "Invalid pair of bodies for applying a position constraint!" << std::endl;
        assert(false);
        return 0;
    }
    
    constraints.push_back(Constraint{{i1, 0, normalize(r1)}, {i2, 0, normalize(r2)}, Constraint::Orientation, vec3(0.0f), d, alpha, 0.0f, false});

    return constraints.size() - 1;
}

void RigidBodySystem::getRigidBodyPositionAndOrientation(const int &i, vec3 &x, vec4 &q) const
{
    if (i >= 0 && i < static_cast<int>(bodies.size()))
    {
        x = bodies[i].x;
        q = bodies[i].q;
    }
}

void RigidBodySystem::getRigidBodyVelocityAndAngularVelocity(const int &i, vec3 &v, vec3 &w) const
{
    if (i >= 0 && i < static_cast<int>(bodies.size()))
    {
        v = bodies[i].v;
        w = bodies[i].w;
    }
}

std::tuple<float, float, float> applyAngularConstraint(const float lambda,
                             const float alpha,
                             RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &dq) noexcept
{
    vec3 n = normalize(dq);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return std::make_tuple(0.0f, 0.0f, 0.0f);

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
        b2->q -= 0.5f*quatmul(vec4(invI2*n, 0.0f), b2->q);
        b2->q = normalize(b2->q);
    }
    
    return std::make_tuple(dlambda, w1, w2);
}

void applyAngularVelocityConstraint(RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &dw) noexcept
{
    vec3 n = normalize(dw);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return;

    const mat3 invI1 = b1->getInvI();
    const mat3 invI2 = b2->getInvI();
    const float w1 = dot(n, invI1*n);
    const float w2 = dot(n, invI2*n);
    
    n *= -length(dw)/(w1 + w2);
    b1->w += invI1*n;
    b2->w -= invI2*n;
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
    
    return std::make_tuple(dlambda, w1, w2);
}

void applyVelocityConstraint(RigidBody *b1,
                             RigidBody *b2,
                             const vec3 &r1,
                             const vec3 &r2,
                             const vec3 &dv) noexcept
{
    vec3 n = normalize(dv);
    
    //Do nothing if we are below numerical precision.
    if (length(n) < 0.99f) return;

    //FIXME: For proper momentum conservation we should have this:
    //const vec3 r1 = p - b1->x;
    //const vec3 r2 = p - b2->x;
    const float w1 = b1->invM + dot(cross(r1, n), b1->getInvI()*cross(r1, n));
    const float w2 = b2->invM + dot(cross(r2, n), b2->getInvI()*cross(r2, n));

    n *= -length(dv)/(w1 + w2);

    b1->v += b1->invM*n;
    b1->w += b1->getInvI()*cross(r1, n);
    
    b2->v -= b2->invM*n;
    b2->w -= b2->getInvI()*cross(r2, n);
}

RigidBodyCollision RigidBodySystem::initializeCollision(RigidBodyCollision c) const noexcept
{
    //Evaluate for currently active body states.
    const RigidBody *b1 = &bodies[c.b1.i];
    const RigidBody *b2 = &bodies[c.b2.i];

    vec4 is;
    vec3 n, p1, p2;
    float d;
    
    if (c.b1.i > 0 && c.b2.i > 0)
    {
        //Get potentially colliding internal spheres.
        is = bodyInternalSpheres[b1->firstInternalSphere + c.b1.sphere];
        const vec4 s1 = vec4(b1->x + mat3::rotationMatrix(b1->q)*is.xyz(), is.w);
        is = bodyInternalSpheres[b2->firstInternalSphere + c.b2.sphere];
        const vec4 s2 = vec4(b2->x + mat3::rotationMatrix(b2->q)*is.xyz(), is.w);

        n = normalize(s2.xyz() - s1.xyz());
        p1 = s1.xyz() + s1.w*n;
        p2 = s2.xyz() - s2.w*n;
        d = dot(s2.xyz() - s1.xyz(), n) - s1.w - s2.w;
    }
    else
    {
        //Case where we have a rigid body colliding with the triangle mesh.
        assert(c.b1.i > 0 && c.b2.i == 0);

        is = bodyInternalSpheres[b1->firstInternalSphere + c.b1.sphere];
        const vec4 s1 = vec4(b1->x + mat3::rotationMatrix(b1->q)*is.xyz(), is.w);
        p2 = getClosestPointOnTriangle(s1.xyz(), c.t);
        
        n = normalize(p2 - s1.xyz());
        p1 = s1.xyz() + s1.w*n;
        d = dot(p2 - s1.xyz(), n) - s1.w;
    }

    c.b1.r = mat3::rotationMatrix(quatconj(b1->q))*(p1 - b1->x);
    c.b2.r = mat3::rotationMatrix(quatconj(b2->q))*(p2 - b2->x);
    c.d = d;
    c.n = n;

    return c;
}

TwoRigidBodyPointGeometry RigidBodySystem::getWorldGeometry(const std::vector<RigidBody> &bodies, const PointOnRigidBody &b1, const PointOnRigidBody &b2) noexcept
{
    const RigidBody *pb1 = &bodies[b1.i];
    const RigidBody *pb2 = &bodies[b2.i];
    const vec3 r1 = mat3::rotationMatrix(pb1->q)*b1.r;
    const vec3 r2 = mat3::rotationMatrix(pb2->q)*b2.r;

    return TwoRigidBodyPointGeometry({pb1->x + r1,
                                       pb2->x + r2,
                                       pb1->v + cross(pb1->w, r1),
                                       pb2->v + cross(pb2->w, r2)});
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
    for (size_t i = 1; i < bodies.size(); ++i)
    {
        const RigidBody &b = bodies[i];

        if (!b.getAABB(dt).isSubsetOf(tree.getNodeBox(i)))
        {
            tree.erase(i);
            tree.insert(b.getAABB(RBAABBDT).scale(RBAABBSCALE), i);
        }
    }

#ifndef NDEBUG
    tree.check();
#endif

    //Check which objects can potentially intersect.
    const auto intersectingObjects = tree.getOverlappingContents();
    std::vector<RigidBodyCollision> collisions;

    //Find all collision points between potentially intersecting Spheres objects.
    //Use simple n^2 comparison (assume small number of internal spheres per object).
    for (const auto &intersection : intersectingObjects)
    {
        //Are collisions allowed between these bodies?
        if (nonCollidingBodies.find(intersection) == nonCollidingBodies.end())
        {
            const RigidBody &b1 = bodies[intersection.first];
            const RigidBody &b2 = bodies[intersection.second];

            //TODO: Use that we could be testing 1 body vs. many other bodies to optimize.
            //TODO: Consider enforcing a fixed number of internal spheres per object to parallellize these checks effectively.
            //TODO: Objects that cannot collide should not be part of the tree in the first place.
            if (b1.canCollide && b2.canCollide) {
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
                            collisions.push_back(RigidBodyCollision({{intersection.first, iS1 - b1.firstInternalSphere, vec3(0.0f)},
                                                                     {intersection.second, iS2 - b2.firstInternalSphere, vec3(0.0f)},
                                                                     0.0f, vec3(0.0f), {vec3(0.0f), vec3(0.0f), vec3(0.0f)}, 0.0f, false}));
                        }
                    }
                }
            }
        }
    }

    //Find all potential collisions between Spheres and triangles.
    for (int iB = 0; iB < static_cast<int>(bodies.size()); ++iB)
    {
        const RigidBody &b = bodies[iB];

        if (b.canCollide)
        {
            const auto triangles = getCollisionTriangles(b);

            for (int i = 0; i < static_cast<int>(triangles.size()) - 2; i += 3)
            {
                const std::array<vec3, 3> t = {{triangles[i], triangles[i + 1], triangles[i + 2]}};

                //Can the body intersect with the triangle?
                if (length(b.x - getClosestPointOnTriangle(b.x, t)) <= b.collisionRadius)
                {
                    const mat3 R = mat3::rotationMatrix(b.q);

                    for (auto iS = b.firstInternalSphere; iS < b.lastInternalSphere; ++iS)
                    {
                        //Get potential collisions taking the object's velocity (linear and angular) into account.
                        vec4 s = vec4(b.x + R*bodyInternalSpheres[iS].xyz(), bodyInternalSpheres[iS].w);

                        s.w += dt*(0.5f*dt*RBMAXACC + length(b.v + cross(b.w, s.xyz() - b.x))) + RBEPS;

                        const vec3 p = getClosestPointOnTriangle(s.xyz(), t);
                        
                        if (length(p - s.xyz()) <= s.w)
                        {
                            //If so, add a potential collision.
                            collisions.push_back(RigidBodyCollision({{iB, iS - b.firstInternalSphere, vec3(0.0f)},
                                                                    {0, 0, vec3(0.0f)},
                                                                    0.0f, vec3(0.0f), t, 0.0f, false}));
                        }
                    }
                }
            }
        }
    }
    
    //Apply forces.
    for (auto &b : bodies)
    {
        b.f = vec3(0.0f);
        b.t = vec3(0.0f);
    }

    applyExternalForces();
    
    //Solve positions.
    const float h = dt/static_cast<float>(nrSubSteps);

    for (int iSubStep = 0; iSubStep < nrSubSteps; ++iSubStep)
    {
        //Initialize non-collision constraints.
        for (auto &c : constraints)
        {
            c.lambda = 0.0f;
            c.forceToZero = false;
        }

        //Store pre-update positions and velocities.
        preBodies = bodies;

        //Apply forces and velocities.
        for (auto &b : bodies)
        {
            if (b.movable)
            {
                b.v += (h*b.invM)*b.f;
                b.x += h*b.v;

                b.w += h*(b.getInvI()*(b.t - cross(b.w, b.getI()*b.w)));
                b.q += (0.5f*h)*quatmul(vec4(b.w, 0.0f), b.q);
                b.q = normalize(b.q);
            }
        }

        //Solve positions.
        
        //Collisions.
        std::random_shuffle(collisions.begin(), collisions.end());

        for (auto &c : collisions)
        {
            c.lambda = 0.0f;
        }

        for (auto &c : collisions)
        {
            //Check whether we have a collision for the current rigid body state.
            c = initializeCollision(c);

            //Check for collision for the current body states.
            if (c.d <= 0.0f || c.forceToZero)
            {
                //Force constraint to be satisfied exactly if it is violated at least once.
                c.forceToZero = true;

                const auto cg = getWorldGeometry(bodies, c.b1, c.b2);
                RigidBody *b1 = &bodies[c.b1.i];
                RigidBody *b2 = &bodies[c.b2.i];

                const float softnessCoeff = 0.5f*(b1->softness + b2->softness)/(h*h);
                
                //Apply position constraint to avoid object interpenetration.
                auto [l, w1, w2] = applyPositionConstraint(c.lambda, softnessCoeff, b1, b2, 0.5f*(cg.p1 + cg.p2), -c.d*c.n);
                
                c.lambda += l;
                
                //Handle static friction.
                const float staticFrictionCoeff = std::sqrt(b1->staticFriction*b2->staticFriction);
                const auto precg = getWorldGeometry(preBodies, c.b1, c.b2);
                
                //Figure out how the current collision point has moved w.r.t. the previous iteration on both bodies.
                //N.B., we need to track a fixed point on the rigid body.
                vec3 dx = (precg.p2 - cg.p2) - (precg.p1 - cg.p1);

                dx -= dot(dx, c.n)*c.n;

                //Inspect normal forces.
                if (c.b2.i == 0 && false)
                {
                    std::cout << "Normal force of " << c.b1.i << ": " << std::abs(c.lambda)/(h*h) << "N, coeff " << staticFrictionCoeff << std::endl;
                    std::cout << "Tangential force of " << c.b1.i << ": " << length(dx)/(h*h*(w1 + w2)) << "N, 1/w1 = " << 1.0f/w1 << "kg, 1/w2 = " << 1.0f/w2 << "kg" << std::endl;
                }

                //Static friction, restrict tangential motion if F_tangential <= mu_static * F_normal.
                if (length(dx)/(w1 + w2) <= staticFrictionCoeff*std::abs(c.lambda))
                {
                    applyPositionConstraint(0.0f, softnessCoeff, b1, b2, 0.5f*(cg.p1 + cg.p2), dx);
                }
            }
        }

        //Apply position constraints.
        //TODO: Randomly shuffle constraints.
        //std::random_shuffle(constraints.begin(), constraints.end());

        for (auto &c : constraints)
        {
            const float softnessCoeff = 0.5f*c.softness/(h*h);
            RigidBody *b1 = &bodies[c.b1.i];
            RigidBody *b2 = &bodies[c.b2.i];
            const vec3 p1 = mat3::rotationMatrix(b1->q)*c.b1.r + b1->x;
            const vec3 p2 = mat3::rotationMatrix(b2->q)*c.b2.r + b2->x;
            //FIXME: How to pick a good point where to apply the constraint?
            const vec3 p = (!b1->movable ? p2 : (!b2->movable ? p1 : 0.5f*(p1 + p2)));
            const vec3 n = mat3::rotationMatrix(b1->q)*c.n;
            const vec3 a = cross(mat3::rotationMatrix(b1->q)*c.b1.r, mat3::rotationMatrix(b2->q)*c.b2.r);
            float d = 0.0f;

            switch (c.type)
            {
                case Constraint::Position:
                    //Position constraint along all axes up to a maximum distance. (For d == 0 this is an equal-position-constraint.)
                    d = length(p2 - p1) - c.d;

                    if (d > 0.0f || c.forceToZero)
                    {
                        auto [l, w1, w2] = applyPositionConstraint(c.lambda, softnessCoeff, b1, b2, p, -d*normalize(p2 - p1));
                        c.forceToZero = true;
                        c.lambda += l;
                    }

                    break;
                case Constraint::PositionOnLine:
                    //Position along a given line up to a maximum distance.

                    //First project delta on the line.
                    if (true)
                    {
                        auto [l, w1, w2] = applyPositionConstraint(c.lambda, softnessCoeff, b1, b2, p, -((p2 - p1) - dot(p2 - p1, n)*n));
                        c.lambda += l;
                    }

                    //Then enforce the maximum extent we can move along the line.
                    d = dot(p2 - p1, n) - c.d;

                    if (d > 0.0f || c.forceToZero)
                    {
                        auto [l, w1, w2] = applyPositionConstraint(c.lambda, softnessCoeff, b1, b2, p, -d*n);
                        c.forceToZero = true;
                        c.lambda += l;
                    }

                    break;
                case Constraint::Orientation:
                    //Constrain relative orientation up to a maximum distance.
                    d = length(a) - c.d;
                    
                    if (d > 0.0f || c.forceToZero)
                    {
                        auto [l, w1, w2] = applyAngularConstraint(c.lambda, softnessCoeff, b1, b2, -d*normalize(a));
                        c.forceToZero = true;
                        c.lambda += l;
                    }

                    break;
                default:
                    break;
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

        //Velocity update for position constraints.
        for (auto &c : constraints)
        {
            if (c.forceToZero)
            {
                const auto cg = getWorldGeometry(bodies, c.b1, c.b2);
                RigidBody *b1 = &bodies[c.b1.i];
                RigidBody *b2 = &bodies[c.b2.i];
                const vec3 n = mat3::rotationMatrix(b1->q)*c.n;
                
                switch (c.type)
                {
                    case Constraint::Position:
                        //Should have relative velocity 0 at a position constraint.
                        applyVelocityConstraint(b1, b2, cg.p1 - b1->x, cg.p2 - b2->x, cg.v1 - cg.v2);
                        break;
                    case Constraint::PositionOnLine:
                        applyVelocityConstraint(b1, b2, cg.p1 - b1->x, cg.p2 - b2->x, dot(cg.v1 - cg.v2, n)*n);
                        break;
                    case Constraint::Orientation:
                        //TODO.
                        break;
                }
            }
        }

        //Velocity update for collisions.
        for (size_t i = 0; i < collisions.size(); ++i)
        {
            const auto c = collisions[i];
            
            //Did we have a collision?
            if (c.forceToZero)
            {
                const auto cg = getWorldGeometry(bodies, c.b1, c.b2);
                RigidBody *b1 = &bodies[c.b1.i];
                RigidBody *b2 = &bodies[c.b2.i];

                //Determine normal and tangential relative velocities.
                vec3 vt = cg.v1 - cg.v2;
                const float vn = dot(vt, c.n);
                
                vt -= vn*c.n;
                
                //Apply dynamic friction.
                const float dynamicFrictionCoeff = std::sqrt(b1->dynamicFriction*b2->dynamicFriction);

                applyVelocityConstraint(b1, b2, cg.p1 - b1->x, cg.p2 - b2->x,
                                        std::min(dynamicFrictionCoeff*std::abs(c.lambda)/h, length(vt))*normalize(vt));
                
                //Perform restitution in case of collisions that are not resting contacts.

                //Get pre-state normal velocity.
                const auto precg = getWorldGeometry(preBodies, c.b1, c.b2);
                const float prevn = dot(precg.v1 - precg.v2, c.n);
                const float restitutionCoeff = (std::abs(prevn) > h*RBMAXACC ? 
                                std::sqrt(b1->restitution*b2->restitution) :
                                0.0f);

                applyVelocityConstraint(b1, b2, cg.p1 - b1->x, cg.p2 - b2->x,
                                        (vn + std::max(0.0f, restitutionCoeff*prevn))*c.n);
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

std::vector<vec3> RigidBodySystem::getCollisionTriangles(const RigidBody &)
{
    //To be implemented by the user, e.g., grab triangles close to the given rigid body and return them.
    //Should include some margin based on the body's velocity.
    return std::vector<vec3>();
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

