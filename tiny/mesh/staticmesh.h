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
#pragma once

#include <string>
#include <vector>

#include <tiny/math/vec.h>

namespace tiny
{

namespace mesh
{

struct StaticMeshVertex
{
    StaticMeshVertex() :
        textureCoordinate(0.0f, 0.0f),
        normal(0.0f, 0.0f, 1.0f),
        position(0.0f, 0.0f, 0.0f)
    {
        
    }
    
    StaticMeshVertex(const vec2 &a_textureCoordinate,
                     const vec3 &a_normal,
                     const vec3 &a_position) :
        textureCoordinate(a_textureCoordinate),
        normal(a_normal),
        position(a_position)
    {

    }

    vec2 textureCoordinate;
    vec3 normal;
    vec3 position;
};

class StaticMesh
{
    public:
        StaticMesh();
        ~StaticMesh();
        
        static StaticMesh createCubeMesh(const float &);
        
        std::vector<StaticMeshVertex> vertices;
        std::vector<unsigned int> indices;
};

}

}

