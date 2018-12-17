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

#include <tiny/math/vec.h>

namespace tiny
{

namespace rigid
{

class RigidBody
{
    public:
        RigidBody();
        RigidBody(const RigidBody &);
        RigidBody(const RigidBody &, const RigidBody &);
        virtual ~RigidBody();

        float getEnergy() const;
        
        static void collide(RigidBody &, RigidBody &, const vec3 &, const float &);

        float inverseMass;
        mat3 inverseInertia;

        vec3 position;
        vec4 orientation;
        vec3 linearMomentum;
        vec3 angularMomentum;
        float elasticity;
};

}

}
