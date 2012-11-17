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
    glDeleteProgram(programIndex);
}

void ShaderProgram::link()
{
    GLint result = GL_FALSE;
    
    glLinkProgram(programIndex);
    glGetProgramiv(programIndex, GL_LINK_STATUS, &result);
    
    if (result == GL_TRUE)
    {
        glValidateProgram(programIndex);
        glGetProgramiv(programIndex, GL_VALIDATE_STATUS, &result);
    }
    
    if (result != GL_TRUE)
    {
        GLint logLength = 0;
        
        glGetProgramiv(programIndex, GL_INFO_LOG_LENGTH, &logLength);
        
        if (logLength <= 0)
        {
            std::cerr << "Unable to link program and there was no info log available!" << std::endl;
            return;
        }
        
        GLchar *logText = new GLchar [logLength + 1];
        
        getProgramInfoLog(programIndex, logLength, 0, logText);
        logText[logLength] = 0;
        
        std::cerr << "Unable to link program!" << std::endl << "Program log:" << std::endl << buffer << std::endl;
        
        delete [] logText;
    }
}

GLuint ShaderProgram::getIndex() const
{
    return programIndex;
}

void ShaderProgram::bind() const
{
    glUseProgram(programIndex);
}

void ShaderProgram::unbind() const
{
    glUseProgram(0);
}

