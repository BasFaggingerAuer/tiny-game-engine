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
    inverseInertia(mat3::identityMatrix()),
    position(0.0f, 0.0f, 0.0f),
    orientation(0.0f, 0.0f, 0.0f, 1.0f),
    linearMomentum(0.0f, 0.0f, 0.0f),
    angularMomentum(0.0f, 0.0f, 0.0f)
{

}

RigidBody::RigidBody(const RigidBody &a) :
    inverseMass(a.inverseMass),
    inverseInertia(a.inverseInertia),
    position(a.position),
    orientation(a.orientation),
    linearMomentum(a.linearMomentum),
    angularMomentum(a.angularMomentum)
{

}

RigidBody::RigidBody(const RigidBody &a, const RigidBody &b) :
    RigidBody()
{
    //Combine two rigid bodies into one larger rigid body for mass and moment of inertia.
    //The orientations of a and b are considered to be fixed w.r.t. each other after this operation.
    
    const mat3 Ia = mat3(a.orientation)*a.inverseInertia.inverted()*mat3(quatconj(a.orientation));
    const mat3 Ib = mat3(b.orientation)*b.inverseInertia.inverted()*mat3(quatconj(b.orientation));
    const vec3 d = b.position - a.position;
    
    inverseMass = 1.0f/((1.0f/a.inverseMass) + (1.0f/b.inverseMass));
    inverseInertia = (Ia + Ib + (1.0f/(a.inverseMass + b.inverseMass))*(length2(d)*mat3::identityMatrix() - mat3::outerProductMatrix(d, d))).inverted();

    position = (b.inverseMass/(b.inverseMass + a.inverseMass))*a.position + (a.inverseMass/(a.inverseMass + b.inverseMass))*b.position;
    orientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    linearMomentum = a.linearMomentum + b.linearMomentum;
    //FIXME: Is this correct?
    angularMomentum = a.angularMomentum + b.angularMomentum;
}

RigidBody::~RigidBody()
{

}

void RigidBody::collide(const RigidBody &a, const RigidBody &b, const vec3 &z, const float &elasticity)
{
    //TODO: Perform collision.
}

