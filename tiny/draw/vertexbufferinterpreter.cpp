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
#include <tiny/draw/glcheck.h>
#include <tiny/draw/vertexbufferinterpreter.h>

using namespace tiny::draw;

VertexBufferInterpreter::VertexBufferInterpreter()
{

}

VertexBufferInterpreter::~VertexBufferInterpreter()
{

}

void VertexBufferInterpreter::bind(const ShaderProgram &program, const size_t &divisor) const
{
    for (std::list<detail::AttributePointerData>::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
    {
        const GLint attributeLocation = glGetAttribLocation(program.getIndex(), i->name.c_str());
        
        if (attributeLocation < 0)
        {
            std::cerr << "Attribute '" << i->name << "' could not be found in the shader program!" << std::endl;
        }
        else
        {
            GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, i->bufferIndex));
            GL_CHECK(glEnableVertexAttribArray(attributeLocation));
            GL_CHECK(glVertexAttribPointer(attributeLocation, i->numComponents, i->type, GL_FALSE, i->stride, (GLvoid *)(i->offset)));
            
            //Enable instanced data if required.
            if (divisor > 0) GL_CHECK(glVertexAttribDivisorARB(attributeLocation, divisor));
        }
    }
    
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBufferInterpreter::unbind(const ShaderProgram &program) const
{
    for (std::list<detail::AttributePointerData>::const_iterator i = attributes.begin(); i != attributes.end(); ++i)
    {
        const GLint attributeLocation = glGetAttribLocation(program.getIndex(), i->name.c_str());
        
        if (attributeLocation >= 0)
        {
            GL_CHECK(glDisableVertexAttribArray(attributeLocation));
        }
    }
}

void VertexBufferInterpreter::addAttribute(const GLuint &bufferIndex, const size_t &numComponents, const GLenum &type, const size_t &stride, const size_t &offset, const std::string &name)
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
    
    attributes.push_back(detail::AttributePointerData(name, bufferIndex, numComponents, type, stride, offset));
}

