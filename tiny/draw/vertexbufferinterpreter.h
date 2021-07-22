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

#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/shaderprogram.h>

namespace tiny
{

namespace draw
{

namespace detail
{

struct AttributePointerData
{
    AttributePointerData(const std::string &a_name,
                         const int &a_numComponents,
                         const GLenum &a_type,
                         const size_t &a_stride,
                         const size_t &a_offset) :
        name(a_name),
        numComponents(a_numComponents),
        type(a_type),
        stride(a_stride),
        offset(a_offset)
    {

    }
    
    std::string name;
    int numComponents;
    GLenum type;
    size_t stride;
    size_t offset;
};

}

/*! \p VertexBufferInterpreter : provides interpretation of vertex buffer data to OpenGL.
 */
template <typename T>
class VertexBufferInterpreter : public VertexBuffer<T>
{
    public:
        VertexBufferInterpreter(const size_t &a_size) :
            VertexBuffer<T>(a_size)
        {

        }
        
        template <typename Iterator>
        VertexBufferInterpreter(Iterator first, Iterator last) :
            VertexBuffer<T>(first, last)
        {

        }
        
        ~VertexBufferInterpreter()
        {

        }
        
        void bind(const ShaderProgram &program, const size_t &divisor = 0) const
        {
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->bufferIndex));
            
            for (std::list<detail::AttributePointerData>::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
            {
                const GLint attributeLocation = glGetAttribLocation(program.getIndex(), i->name.c_str());
                
                if (attributeLocation < 0)
                {
                    std::cerr << "Attribute '" << i->name << "' could not be found in the shader program!" << std::endl;
                }
                else
                {
                    GL_CHECK(glEnableVertexAttribArray(attributeLocation));
                    
                    if (i->type == GL_INT) GL_CHECK(glVertexAttribIPointer(attributeLocation, i->numComponents, i->type, i->stride, (GLvoid *)(i->offset)));
                    else GL_CHECK(glVertexAttribPointer(attributeLocation, i->numComponents, i->type, GL_FALSE, i->stride, (GLvoid *)(i->offset)));
                    
                    //Enable instanced data if required.
                    if (divisor > 0) GL_CHECK(glVertexAttribDivisor(attributeLocation, divisor));
                }
            }
            
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        void unbind(const ShaderProgram &program) const
        {
            for (std::list<detail::AttributePointerData>::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
            {
                const GLint attributeLocation = glGetAttribLocation(program.getIndex(), i->name.c_str());
                
                if (attributeLocation >= 0)
                {
                    GL_CHECK(glVertexAttribDivisor(attributeLocation, 0));
                    GL_CHECK(glDisableVertexAttribArray(attributeLocation));
                }
            }
        }
        
    protected:
        void addFloatAttribute(const size_t &offset, const std::string &name) {addAttribute(1, GL_FLOAT, sizeof(T), offset, name);}
        void addVec2Attribute(const size_t &offset, const std::string &name) {addAttribute(2, GL_FLOAT, sizeof(T), offset, name);}
        void addVec3Attribute(const size_t &offset, const std::string &name) {addAttribute(3, GL_FLOAT, sizeof(T), offset, name);}
        void addVec4Attribute(const size_t &offset, const std::string &name) {addAttribute(4, GL_FLOAT, sizeof(T), offset, name);}
        void addIVec2Attribute(const size_t &offset, const std::string &name) {addAttribute(2, GL_INT, sizeof(T), offset, name);}
        void addIVec3Attribute(const size_t &offset, const std::string &name) {addAttribute(3, GL_INT, sizeof(T), offset, name);}
        void addIVec4Attribute(const size_t &offset, const std::string &name) {addAttribute(4, GL_INT, sizeof(T), offset, name);}
        
    private:
        void addAttribute(const size_t &numComponents, const GLenum &type, const size_t &stride, const size_t &offset, const std::string &name)
        {
            bool found = false;
            
            for (std::list<detail::AttributePointerData>::const_iterator i = attributes.begin(); i != attributes.end() && !found; ++i)
            {
                if (i->name == name) found = true;
            }
            
            if (found || name.empty())
            {
                std::cerr << "Attribute '" << name << "' already exists!" << std::endl;
                return;
            }
            
            attributes.push_back(detail::AttributePointerData(name, numComponents, type, stride, offset));
        }
        
        std::list<detail::AttributePointerData> attributes;
};

}

}

