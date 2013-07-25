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
#include <iostream>
#include <exception>

#include <tiny/draw/shaderprogram.h>

using namespace tiny::draw;

ShaderProgram::ShaderProgram()
{
    programIndex = glCreateProgram();
    
    if (programIndex == 0)
        throw std::bad_alloc();
}

ShaderProgram::ShaderProgram(const ShaderProgram &)
{

}

ShaderProgram::~ShaderProgram()
{
    assert(programIndex != 0);
    GL_CHECK(glDeleteProgram(programIndex));
}

void ShaderProgram::link()
{
    GLint result = GL_FALSE;
    
    GL_CHECK(glLinkProgram(programIndex));
    GL_CHECK(glGetProgramiv(programIndex, GL_LINK_STATUS, &result));
    
    if (result != GL_TRUE)
    {
        GLint logLength = 0;
        
        GL_CHECK(glGetProgramiv(programIndex, GL_INFO_LOG_LENGTH, &logLength));
        
        if (logLength <= 0)
        {
            std::cerr << "Unable to link program and there was no info log available!" << std::endl;
            return;
        }
        
        GLchar *logText = new GLchar [logLength + 1];
        
        GL_CHECK(glGetProgramInfoLog(programIndex, logLength, 0, logText));
        logText[logLength] = 0;
        
        std::cerr << "Unable to link program!" << std::endl << "Program log:" << std::endl << logText << std::endl;
        
        delete [] logText;
        
#ifndef NDEBUG
        throw std::exception();
#endif
    }
#ifndef NDEBUG
    else
    {
        std::cerr << "Successfully linked shader program " << programIndex << "." << std::endl;
    }
#endif
}

bool ShaderProgram::validate() const
{
    GLint result = GL_FALSE;
    
    GL_CHECK(glValidateProgram(programIndex));
    GL_CHECK(glGetProgramiv(programIndex, GL_VALIDATE_STATUS, &result));
    
    if (result != GL_TRUE)
    {
        GLint logLength = 0;
        
        GL_CHECK(glGetProgramiv(programIndex, GL_INFO_LOG_LENGTH, &logLength));
        
        if (logLength <= 0)
        {
            std::cerr << "Unable to validate program and there was no info log available!" << std::endl;
            return false;
        }
        
        GLchar *logText = new GLchar [logLength + 1];
        
        GL_CHECK(glGetProgramInfoLog(programIndex, logLength, 0, logText));
        logText[logLength] = 0;
        
        std::cerr << "Unable to validate program " << programIndex << "!" << std::endl << "Program log:" << std::endl << logText << std::endl;
        
        delete [] logText;
        
        return false;
    }
#ifndef NDEBUG
    else
    {
        std::cerr << "Successfully validated shader program " << programIndex << "." << std::endl;
    }
#endif
    
    return true;
}

GLuint ShaderProgram::getIndex() const
{
    return programIndex;
}

void ShaderProgram::bind() const
{
    GL_CHECK(glUseProgram(programIndex));
}

void ShaderProgram::unbind() const
{
    GL_CHECK(glUseProgram(0));
}

