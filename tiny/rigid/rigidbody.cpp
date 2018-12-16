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

#include <tiny/rigid/rigidbody.h>

using namespace tiny;
using namespace tiny::rigid;

RigidBody::RigidBody() :
    inverseMass(1.0f),
    inverseInertia(mat4::identityMatrix()),
    position(0.0f, 0.0f, 0.0f),
    orientation(0.0f, 0.0f, 0.0f, 1.0f),
    linearMomentum(0.0f, 0.0f, 0.0f),
    angularMomentum(0.0f, 0.0f, 0.0f),
    elasticity(1.0f)
{

}

RigidBody::RigidBody(const RigidBody &a) :
    inverseMass(a.inverseMass),
    inverseInertia(a.inverseInertia),
    position(a.position),
    orientation(a.orientation),
    linearMomentum(a.linearMomentum),
    angularMomentum(a.angularMomentum),
    elasticity(a.elasticity)
{

}

RigidBody::RigidBody(const RigidBody &a, const RigidBody &b)
{
    //Combine two rigid bodies into one larger rigid body.
    inverseMass = 1.0f/((1.0f/a.inverseMass) + (1.0f/b.inverseMass));
    //TODO: Finish this.
}

RigidBody::~RigidBody()
{

}

void RigidBody::collide(const RigidBody &a, const RigidBody &b, const vec3 &z)
{
    //TODO: Perform collision.
}

