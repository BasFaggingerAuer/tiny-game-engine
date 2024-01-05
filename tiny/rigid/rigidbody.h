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
#include <set>

#include <tiny/math/vec.h>
#include <tiny/rigid/aabbtree.h>

#include <tiny/draw/staticmeshhorde.h>

namespace tiny
{

namespace rigid
{

//TODO: Structure of arrays instead of array of structures.
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
    bool canCollide; // Can the object collide with other objects at all?
    
    //Body sphere geometry.
    float radius; //Size of rigid body.
    float collisionRadius; //With margin for collision detection.
    int firstInternalSphere; //Index of first internal sphere.
    int lastInternalSphere; //Index of last internal sphere.

    //TODO: Move to material class.
    float staticFriction; //Static friction coefficient.
    float dynamicFriction; //Dynamic friction coefficient.
    float restitution; //Restitution coefficient for collisions.
    float softness; //Inverse material hardness.
    
    mat3 getI() const noexcept
    {
        return mat3::rotationMatrix(q)*mat3::scaleMatrix(1.0f/invI)*mat3::rotationMatrix(quatconj(q));
    }

    mat3 getInvI() const noexcept
    {
        return mat3::rotationMatrix(q)*mat3::scaleMatrix(invI)*mat3::rotationMatrix(quatconj(q));
    }

    aabb::aabb getAABB(const float &) const noexcept;

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

struct PointOnRigidBody
{
    int i; //Index of rigid body.
    int sphere; //Index of internal sphere (for collisions).
    vec3 r; //Point in body's local coordinates.
};

struct TwoRigidBodyPointGeometry
{
    vec3 p1, p2; //Positions of colliding points in world coordinate system.
    vec3 v1, v2; //Velocities of colliding points in world coorindate system.
};

struct RigidBodyCollision
{
    PointOnRigidBody b1, b2;
    float d; //Signed distance of b2 w.r.t. b1 along the collision normal. A collision occurs if d <= 0.
    vec3 n; //Normal of collision surface.
    std::array<vec3, 3> t; //Triangle in case b2.i == 0.
    float lambda; //Constraint multiplier.
    bool forceToZero; //Force constraint to equality if it has been violated at least once.
};

struct Constraint
{
    PointOnRigidBody b1, b2;

    enum
    {
        Position,
        PositionOnLine,
        Orientation
    } type; //Constraint type.

    vec3 n; //Constraint direction (if needed, based on type).
    float d; //Maximum allowed distance until constraint is enforced.
    
    float softness; //Constraint inverse stiffness.
    float lambda;
    bool forceToZero; //Force constraint to equality if it has been violated at least once.
};

class RigidBodySystem
{
    public:
        RigidBodySystem(const int & = 4);
        virtual ~RigidBodySystem();
        
        int addImmovableSpheresRigidBody(const std::vector<vec4> &, const vec3 &,
            const float & = 0.6f, const float & = 0.5f, const float & = 0.7f, const float & = 0.0f); //Friction/restitution ~steel/aluminum.

        int addSpheresRigidBody(const float &, const std::vector<vec4> &,
            const vec3 &, const vec3 & = vec3(0.0f, 0.0f, 0.0f),
            const vec4 & = vec4(0.0f, 0.0f, 0.0f, 1.0f), const vec3 & = vec3(0.0f, 0.0f, 0.0f),
            const float & = 0.6f, const float & = 0.5f, const float & = 0.7f, const float & = 0.0f); //Friction/restitution ~steel/aluminum.
        //TODO: Ability to remove rigid bodies.

        void addNonCollidingPair(const int &, const int &);
        int addPositionConstraint(const int &, const vec3 &, const int &, const vec3 &, const float & = 0.0f, const float & = 0.0f);
        int addPositionLineConstraint(const int &, const vec3 &, const vec3 &, const int &, const vec3 &, const float & = 0.0f, const float & = 0.0f);
        int addAngularConstraint(const int &, const vec3 &, const int &, const vec3 &, const float & = 0.0f, const float & = 0.0f);

        void getRigidBodyPositionAndOrientation(const int &, vec3 &, vec4 &) const;
        void getRigidBodyVelocityAndAngularVelocity(const int &, vec3 &, vec3 &) const;
        
        void update(const float &);
        
        //Ability to export all internal spheres for rendering in a static mesh horde.
        //Container should contain a StaticMeshInstance-compatible type.
        template <typename Container>
        void getInternalSphereStaticMeshes(Container &out) const noexcept
        {
            for (const auto &b : bodies)
            {
                const mat3 R = mat3::rotationMatrix(b.q);
                
                for (int i = b.firstInternalSphere; i < b.lastInternalSphere; ++i)
                {
                    const vec4 s = bodyInternalSpheres[i];

                    out.push_back(tiny::draw::StaticMeshInstance(vec4(b.x + R*s.xyz(), s.w), b.q));
                }
            }
        }

        template <typename Container, typename List>
        void getStaticMeshes(Container &out, const List &in) const noexcept
        {
            for (const auto &i : in)
            {
                out.push_back(tiny::draw::StaticMeshInstance(vec4(bodies[i].x, 1.0f), bodies[i].q));
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
        virtual std::vector<vec3> getCollisionTriangles(const RigidBody &);
    
        float time;
        float totalEnergy;
        vec3 totalLinearMomentum;
        vec3 totalAngularMomentum;

        std::vector<RigidBody> bodies;
        std::vector<Constraint> constraints;        
        
        const int nrSubSteps;

    private:
        std::vector<RigidBody> preBodies;
        aabb::Tree tree;
        std::vector<vec4> bodyInternalSpheres;
        std::vector<vec4> collSpheres;
        std::set<std::pair<int, int>> nonCollidingBodies;
        
        void calculateInternalSpheres(const RigidBody &, const float &);
        RigidBodyCollision initializeCollision(RigidBodyCollision) const noexcept;
        float addMarginToRadius(const float, const float) const;
        static TwoRigidBodyPointGeometry getWorldGeometry(const std::vector<RigidBody> &, const PointOnRigidBody &, const PointOnRigidBody &) noexcept;
};

}

}

