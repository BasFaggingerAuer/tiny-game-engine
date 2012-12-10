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

#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>

namespace tiny
{

namespace draw
{

namespace detail
{

struct TerrainBlockInstance
{
    TerrainBlockInstance() :
        scaleAndTranslate(vec4(1.0f, 1.0f, 0.0f, 0.0f))
    {

    }

    TerrainBlockInstance(const vec4 &a_scaleAndTranslate) :
        scaleAndTranslate(a_scaleAndTranslate)
    {

    }
    
    vec4 scaleAndTranslate;
};

class TerrainBlockVertexBufferInterpreter : public VertexBufferInterpreter<vec2>
{
    public:
        TerrainBlockVertexBufferInterpreter(const size_t &, const size_t &);
        ~TerrainBlockVertexBufferInterpreter();
};

class TerrainBlockInstanceBufferInterpreter : public VertexBufferInterpreter<TerrainBlockInstance>
{
    public:
        TerrainBlockInstanceBufferInterpreter(const size_t &);
        ~TerrainBlockInstanceBufferInterpreter();
};

class TerrainBlockIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        TerrainBlockIndexBuffer(const size_t &, const size_t &);
        ~TerrainBlockIndexBuffer();
};

class TerrainBlock
{
    public:
        TerrainBlock(const size_t &, const size_t &, const size_t &);
        ~TerrainBlock();
        
        TerrainBlockInstance & operator [] (const size_t &);
        const TerrainBlockInstance & operator [] (const size_t &) const;
        
        void bind(const ShaderProgram &) const;
        void unbind(const ShaderProgram &) const;
        
        TerrainBlockVertexBufferInterpreter vertices;
        TerrainBlockInstanceBufferInterpreter instances;
        TerrainBlockIndexBuffer indices;
};

}

}

}

