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
    vec4 scaleAndTranslate;
};

class TerrainBlockVertexBuffer : public VertexBuffer<vec2>
{
    public:
        TerrainBlockVertexBuffer(const size_t &);
        ~TerrainBlockVertexBuffer();
};

class TerrainBlockVertexBufferInterpreter : public VertexBufferInterpreter
{
    public:
        TerrainBlockVertexBufferInterpreter(const size_t &, const size_t &);
        ~TerrainBlockVertexBufferInterpreter();
        
    private:
        friend class TerrainBlock;
        
        TerrainBlockVertexBuffer vertices;
        VertexBuffer<TerrainBlockInstance> instances;
};

class TerrainBlockIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        TerrainBlockIndexBuffer(const size_t &);
        ~TerrainBlockIndexBuffer();
};

class TerrainBlock
{
    public:
        TerrainBlock(const size_t &, const size_t &);
        ~TerrainBlock();
        
        TerrainBlockInstance & operator [] (const size_t &);
        const TerrainBlockInstance & operator [] (const size_t &) const;
        
        void bind(const ShaderProgram &) const;
        void unbind(const ShaderProgram &) const;
        
        TerrainBlockVertexBufferInterpreter vertices;
        TerrainBlockIndexBuffer indices;
};

class TerrainStitchVertexBuffer : public VertexBuffer<vec2>
{
    public:
        TerrainStitchVertexBuffer(const size_t &);
        ~TerrainStitchVertexBuffer();
};

class TerrainStitchIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        TerrainStitchIndexBuffer(const size_t &);
        ~TerrainStitchIndexBuffer();
};

class TerrainStitchVertexBufferInterpreter : public VertexBufferInterpreter
{
    public:
        TerrainStitchVertexBufferInterpreter(const size_t &, const size_t &);
        ~TerrainStitchVertexBufferInterpreter();
        
    private:
        TerrainStitchVertexBuffer vertices;
        VertexBuffer<TerrainBlockInstance> instances;
};

}

}

}

