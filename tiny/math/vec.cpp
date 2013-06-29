/*
Copyright 2012, Bas Fagginger Auer.

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
#include <cstdlib>

#include <tiny/math/vec.h>

using namespace tiny;

//Quaternion specific operations.
vec4 tiny::quatmul(const vec4 &a, const vec4 &b)
{
    return vec4(a.w*b.x + b.w*a.x + a.y*b.z - a.z*b.y,
        a.w*b.y + b.w*a.y + a.z*b.x - a.x*b.z,
        a.w*b.z + b.w*a.z + a.x*b.y - a.y*b.x,
        a.w*b.w - (a.x*b.x + a.y*b.y + a.z*b.z));
}

vec4 tiny::quatconj(const vec4 &a)
{
    return vec4(-a.x, -a.y, -a.z, a.w);
}

void tiny::quatnormalize(vec4 &a)
{
    const float l = 1.0/sqrt(a.x*a.x + a.y*a.y + a.z*a.z + a.w*a.w);
    
    a.x *= l; a.y *= l; a.z *= l; a.w *= l;
}

vec4 tiny::quatrot(const float &alpha, const vec3 &a)
{
    const float s = sin(0.5*alpha);
    
    return vec4(s*a.x, s*a.y, s*a.z, cos(0.5*alpha));
}

vec4 tiny::quatmatrix(const mat4 &a)
{
    //From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm.
    const float t = a.v00 + a.v11 + a.v22;
    vec4 q;
    
    if (t > 0.0f)
    {
        const float s = 0.5f/sqrtf(1.0f + t);
        
        q = vec4((a.v21 - a.v12)*s, (a.v02 - a.v20)*s, (a.v10 - a.v01)*s, 0.25f/s);
    }
    else
    {
        if (a.v00 > a.v11 && a.v00 > a.v22)
        {
            const float s = 0.5f/sqrtf(1.0f + a.v00 - a.v11 - a.v22);
            
            q = vec4(0.25f/s, (a.v01 + a.v10)*s, (a.v02 + a.v20)*s, (a.v21 - a.v12)*s);
        }
        else if (a.v11 > a.v22)
        {
            const float s = 0.5f/sqrtf(1.0f + a.v11 - a.v00 - a.v22);
            
            q = vec4((a.v01 + a.v10)*s, 0.25f/s, (a.v12 + a.v21)*s, (a.v02 - a.v20)*s);
        }
        else
        {
            const float s = 0.5f/sqrtf(1.0f + a.v22 - a.v00 - a.v11);
            
            q = vec4((a.v02 + a.v20)*s, (a.v12 + a.v21)*s, 0.25f/s, (a.v10 - a.v01)*s);
        }
    }
    
    return normalize(q);
}

vec2 tiny::randomVec2(const float &s)
{
    return vec2(2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s);
}

vec3 tiny::randomVec3(const float &s)
{
    return vec3(2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s);
}

vec4 tiny::randomVec4(const float &s)
{
    return vec4(2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s,
                2.0f*s*static_cast<float>(rand())/static_cast<float>(RAND_MAX) - s);
}

