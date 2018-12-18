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

float RigidBody::getEnergy() const
{
    //Calculate energy of this rigid body.
    const mat3 invI = mat3(orientation)*inverseInertia*mat3(quatconj(orientation));

    return 0.5f*inverseMass*length2(linearMomentum) + 0.5f*dot(angularMomentum, invI*angularMomentum);
}

void RigidBody::collide(RigidBody &a, RigidBody &b, const vec3 &z, const float &elasticity)
{
    //Perform collision between two rigid bodies, conserving linear momentum, angular momentum and energy if the elasticity == 1.0.
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

RigidBox::RigidBox(const float &mass, const vec3 &dimensions, const vec3 &_position, const vec4 &_orientation) :
    RigidBody()
{
    position = _position;
    orientation = _orientation;
    inverseMass = 1.0f/mass;

    inverseInertia = mat3::identityMatrix();
    inverseInertia.v00 = 12.0f/(mass*(dimensions.y*dimensions.y + dimensions.z*dimensions.z));
    inverseInertia.v11 = 12.0f/(mass*(dimensions.x*dimensions.x + dimensions.z*dimensions.z));
    inverseInertia.v22 = 12.0f/(mass*(dimensions.x*dimensions.x + dimensions.y*dimensions.y));
}

RigidBox::~RigidBox()
{

}

RigidSphere::RigidSphere(const float &mass, const float &radius, const vec3 &_position, const vec4 &_orientation) :
    RigidBody()
{
    position = _position;
    orientation = _orientation;
    inverseMass = 1.0f/mass;

    inverseInertia = mat3::identityMatrix();
    inverseInertia.v00 = 5.0f/(2.0f*mass*radius*radius);
    inverseInertia.v11 = 5.0f/(2.0f*mass*radius*radius);
    inverseInertia.v22 = 5.0f/(2.0f*mass*radius*radius);
}

RigidSphere::~RigidSphere()
{

}

RigidTube::RigidTube(const float &mass, const float &innerRadius, const float &outerRadius, const float &length, const vec3 &_position, const vec4 &_orientation) :
    RigidBody()
{
    position = _position;
    orientation = _orientation;
    inverseMass = 1.0f/mass;

    inverseInertia = mat3::identityMatrix();
    inverseInertia.v00 = 12.0f/(mass*(3.0f*(innerRadius*innerRadius + outerRadius*outerRadius) + length*length));
    inverseInertia.v11 = 12.0f/(mass*(3.0f*(innerRadius*innerRadius + outerRadius*outerRadius) + length*length));
    inverseInertia.v22 =  2.0f/(mass*(innerRadius*innerRadius + outerRadius*outerRadius));
}

RigidTube::~RigidTube()
{

}


