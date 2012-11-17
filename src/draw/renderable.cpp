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
#include <algorithm>

#include <draw/renderable.h>

using namespace tiny::draw;

Renderable::Renderable()
{

}

Renderable::Renderable(const Renderable &a_renderable)
{
    //TODO.
}

Renderable::~Renderable()
{

}

std::string Renderable::getGeometryShaderCode() const
{
    return "";
}

void Renderable::setVariablesInProgram(ShaderProgram &program) const
{
    program.bind();
    
    for (map<std::string, FloatUniform>::const_iterator i = floatUniforms.begin(); i != floatUniforms.end(); ++i)
    {
        const BoundUniform uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        
        if (location < 0)
        {
            std::cerr << "Warning: uniform variable '" << uniform.name << "' does not exist in the GLSL program!" << std::endl;
            continue;
        }
        
             if (uniform.numParameters == 1) glUniform1f(location, uniform.x);
        else if (uniform.numParameters == 2) glUniform2f(location, uniform.x, uniform.y);
        else if (uniform.numParameters == 3) glUniform3f(location, uniform.x, uniform.y, uniform.z);
        else if (uniform.numParameters == 4) glUniform4f(location, uniform.x, uniform.y, uniform.z, uniform.w);
        else std::cerr << "Warning: uniform variable '" << uniform.name << "' has an invalid number of parameters (" << uniform.numParameters << ")!" << std::endl;
    }
    
    int textureBindPoint = 0;
    
    for (map<std::string, BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        const BoundTexture uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        
        if (location < 0)
        {
            std::cerr << "Warning: texture '" << uniform.name << "' does not exist in the GLSL program!" << std::endl;
            continue;
        }
        
        glUniform1i(location, textureBindPoint++);
    }
    
    program.unbind();
}

void Renderable::bindTextures() const
{
    int textureBindPoint = 0;
    
    for (map<std::string, BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        i->second.texture->bind(textureBindPoint++);
    }
}

void Renderable::unbindTextures() const
{
    int textureBindPoint = 0;
    
    for (map<std::string, BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        i->second.texture->unbind(textureBindPoint++);
    }
}

void Renderable::setFloatVariable(const float &x, const std::string &name) {floatUniforms[name] = FloatUniform(name, 1, x);}
void Renderable::setVec2Variable(const float &x, const float &y, const std::string &name) {floatUniforms[name] = FloatUniform(name, 2, x, y);}
void Renderable::setVec3Variable(const float &x, const float &y, const float &z, const std::string &name) {floatUniforms[name] = FloatUniform(name, 3, x, y, z);}
void Renderable::setVec4Variable(const float &x, const float &y, const float &z, const float &w, const std::string &name) {floatUniforms[name] = FloatUniform(name, 4, x, y, z, w);}

