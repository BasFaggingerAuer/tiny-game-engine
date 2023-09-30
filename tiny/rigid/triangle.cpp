/*
Copyright 2023, Bas Fagginger Auer.

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
#include <tiny/rigid/triangle.h>

using namespace tiny;

vec3 tiny::getClosestPointOnTriangle(const vec3 &p, const std::array<vec3, 3> &t) noexcept
{
    //Determine closest point on a triangle a-b-c w.r.t. a given point p.
    //From Fast Distance Queries for Triangles, Lines, and Points using SSE Instructions by E. Shellshear, 2014.
    const vec3 ab = t[1] - t[0];
    const vec3 ac = t[2] - t[0];
    const vec3 ap = p - t[0];
    const float d1 = dot(ab, ap);
    const float d2 = dot(ac, ap);

    //Are we closest to point a?
    if (d1 <= 0.0f && d2 <= 0.0f) return t[0];

    const vec3 bp = p - t[1];
    const float d3 = dot(ab, bp);
    const float d4 = dot(ac, bp);

    //Are we closest to point b?
    if (d3 >= 0.0f && d4 <= d3) return t[1];
    
    const vec3 cp = p - t[2];
    const float d5 = dot(ab, cp);
    const float d6 = dot(ac, cp);

    //Are we closest to point c?
    if (d6 >= 0.0f && d5 <= d6) return t[2];
    
    const float vc = d1*d4 - d3*d2;
    const float v1 = d1/(d1 - d3);

    //Are we closest to line ab?
    if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) return t[0] + v1*ab;
    
    const float vb = d5*d2 - d1*d6;
    const float w1 = d2/(d2 - d6);

    //Are we closest to line ac?
    if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) return t[0] + w1*ac;

    const float va = d3*d6 - d5*d4;
    const float w2 = (d4 - d3)/((d4 - d3) + (d5 - d6));

    //Are we closest to line bc?
    if (va <= 0.0f && d4 - d3 >= 0.0f && d5 - d6 >= 0.0f) return t[1] + w2*(t[2] - t[1]);
    
    const float denom = 1.0f/(va + vb + vc);
    const float v2 = vb*denom;
    const float w3 = vc*denom;
    
    //We are closest to an interior point.
    return t[0] + v2*ab + w3*ac;
}

