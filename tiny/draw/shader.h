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
#include <vector>

#include <cassert>

#include <GL/glew.h>
#include <GL/gl.h>

#include <tiny/draw/glcheck.h>

namespace tiny
{

namespace draw
{

template<GLuint ShaderType>
class Shader
{
    public:
        Shader()
        {
            shaderIndex = glCreateShader(ShaderType);
            
            if (shaderIndex == 0)
                throw std::bad_alloc();
        }
        
        virtual ~Shader()
        {
            assert(shaderIndex != 0);
            GL_CHECK(glDeleteShader(shaderIndex));
        }
        
        GLuint getIndex() const
        {
            return shaderIndex;
        }
        
        void compile(const std::string &a_code)
        {
            const char *code = a_code.c_str();
            GLint length = a_code.size();
            GLint results = GL_FALSE;
            
            GL_CHECK(glShaderSource(shaderIndex, 1, (const GLchar **)(&code), &length));
            GL_CHECK(glCompileShader(shaderIndex));
            GL_CHECK(glGetShaderiv(shaderIndex, GL_COMPILE_STATUS, &results));
            
            if (results != GL_TRUE)
            {
                GLint logLength = 0;
                
                GL_CHECK(glGetShaderiv(shaderIndex, GL_INFO_LOG_LENGTH, &logLength));
                
                if (logLength <= 0)
                {
                    std::cerr << "Unable to compile program and there was no info log available!" << std::endl;
                    return;
                }
                
                GLchar *logText = new GLchar [logLength + 1];
                
                GL_CHECK(glGetShaderInfoLog(shaderIndex, logLength, 0, logText));
                logText[logLength] = 0;
                
                std::cerr << "Unable to compile shader:" << std::endl << a_code << std::endl << "Shader log:" << std::endl << logText << std::endl;
                
                delete [] logText;
                
#ifndef NDEBUG
                throw std::exception();
#endif
            }
#ifndef NDEBUG
            else
            {
                //std::cerr << "Successfully compiled shader " << shaderIndex << ":" << std::endl << a_code << std::endl;
                std::cerr << "Successfully compiled shader " << shaderIndex << "." << std::endl;
            }
#endif
        }
    
    private:
        Shader(const Shader &)
        {

        }
        
        GLuint shaderIndex;
};

class VertexShader : public Shader<GL_VERTEX_SHADER>
{
    public:
        VertexShader();
        ~VertexShader();
};

class GeometryShader : public Shader<GL_GEOMETRY_SHADER>
{
    public:
        GeometryShader();
        ~GeometryShader();
};

class FragmentShader : public Shader<GL_FRAGMENT_SHADER>
{
    public:
        FragmentShader();
        ~FragmentShader();
};

}

}

