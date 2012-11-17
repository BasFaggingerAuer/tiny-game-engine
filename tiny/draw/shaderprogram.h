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

#include <exception>
#include <vector>

#include <cassert>

#include <tiny/draw/shader.h>

namespace tiny
{

namespace draw
{

/*! \p ShaderProgram : GLSL program describing the transformation of vertices and pixel data.
 */
class ShaderProgram
{
    public:
        ShaderProgram();
        ~ShaderProgram();
        
        template<GLuint ShaderType>
        void attach(const Shader<ShaderType> &shader)
        {
            glAttachShader(programIndex, shader.getIndex());
        }

        template<GLuint ShaderType>
        void detach(const Shader<ShaderType> &shader)
        {
            glDetachShader(programIndex, shader.getIndex());
        }

        void link();
        GLuint getIndex() const;
        void bind() const;
        void unbind() const;
        
    private:
        ShaderProgram(const ShaderProgram &);
        
        GLuint programIndex;
};

}

}

