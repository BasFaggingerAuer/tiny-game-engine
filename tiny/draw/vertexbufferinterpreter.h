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

#include <iostream>
#include <exception>
#include <cassert>
#include <list>
#include <string>

#include <GL/gl.h>

#include <draw/vertexbuffer.h>
#include <draw/shaderprogram.h>

namespace tiny
{

namespace draw
{

namespace detail
{

struct AttributePointerData
{
    AttributePointerData(const std::string &a_name,
                         const GLuint &a_bufferIndex,
                         const int &a_numComponents,
                         const GLenum &a_type,
                         const size_t &a_stride,
                         const size_t &a_offset) :
        name(a_name),
        bufferIndex(a_bufferIndex),
        numComponents(a_numComponents),
        type(a_type),
        stride(a_stride),
        offset(a_offset)
    {

    }
    
    std::string name;
    GLuint bufferIndex;
    int numComponents;
    GLenum type;
    size_t stride;
    size_t offset;
};

}

/*! \p VertexBufferInterpreter : provides interpretation of vertex buffer data to OpenGL.
 */
class VertexBufferInterpreter
{
    public:
        VertexBufferInterpreter();
        ~VertexBufferInterpreter();
        
        void bind(const ShaderProgram &, const size_t & = 0) const;
        void unbind(const ShaderProgram &) const;
        
    protected:
        template<typename T> void addFloatAttribute(const VertexBuffer<T> &buffer, const size_t &offset, const std::string &name) {addAttribute(buffer.getIndex(), 1, GL_FLOAT, sizeof(T), offset, name);}
        template<typename T> void addVec2Attribute(const VertexBuffer<T> &buffer, const size_t &offset, const std::string &name) {addAttribute(buffer.getIndex(), 2, GL_FLOAT, sizeof(T), offset, name);}
        template<typename T> void addVec3Attribute(const VertexBuffer<T> &buffer, const size_t &offset, const std::string &name) {addAttribute(buffer.getIndex(), 3, GL_FLOAT, sizeof(T), offset, name);}
        template<typename T> void addVec4Attribute(const VertexBuffer<T> &buffer, const size_t &offset, const std::string &name) {addAttribute(buffer.getIndex(), 4, GL_FLOAT, sizeof(T), offset, name);}
        
    private:
        void addAttribute(const GLuint &, const size_t &, const GLenum &, const size_t &, const size_t &, const std::string &);
        
        std::list<detail::AttributePointerData> attributes;
};

}

}

