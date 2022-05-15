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
#include <tiny/math/genmat.h>

using namespace tiny;

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

vec2 tiny::to_float(const ivec2 &a) noexcept
{
    return vec2(static_cast<float>(a.x), static_cast<float>(a.y));
}

vec3 tiny::to_float(const ivec3 &a) noexcept
{
    return vec3(static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z));
}

vec4 tiny::to_float(const ivec4 &a) noexcept
{
    return vec4(static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z), static_cast<float>(a.w));
}

ivec2 tiny::to_int(const vec2 &a) noexcept
{
    return ivec2(static_cast<int>(a.x), static_cast<int>(a.y));
}

ivec3 tiny::to_int(const vec3 &a) noexcept
{
    return ivec3(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(a.z));
}

ivec4 tiny::to_int(const vec4 &a) noexcept
{
    return ivec4(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(a.z), static_cast<int>(a.w));
}

std::tuple<vec3, mat3> mat3::eigenDecompositionSym() const
{
    const auto [e, E] = tiny::eigenDecompositionSym<float, 3>(fromFixedSizeMatrix(*this));
    
    return {toFixedSizeVector(e), toFixedSizeMatrix(E)};
}


