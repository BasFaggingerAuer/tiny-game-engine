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
#include <map>
#include <algorithm>

#include <tiny/draw/uniformmap.h>

using namespace tiny::draw;

UniformMap::UniformMap() :
    texturesAreLocked(false)
{

}

UniformMap::~UniformMap()
{

}

size_t UniformMap::getNrUniforms() const
{
    return intUniforms.size() + floatUniforms.size() + matrixUniforms.size();
}

size_t UniformMap::getNrTextures() const
{
    return textures.size();
}

void UniformMap::addTexture(const std::string &name)
{
    if (texturesAreLocked)
    {
        std::cerr << "Warning: attempting to add texture '" << name << "' to a locked uniform map!" << std::endl;
        return;
    }
    
    if (textures.find(name) != textures.end())
    {
        std::cerr << "Warning: texture '" << name << "' already exists!" << std::endl;
        return;
    }
    
    textures[name] = detail::BoundTexture(name, 0);
}

void UniformMap::lockTextures()
{
    texturesAreLocked = true;
}

void UniformMap::setIntUniform(const int &x, const std::string &name) {intUniforms[name] = detail::IntUniform(name, 1, x);}

void UniformMap::setFloatUniform(const float &x, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 1, x);}
void UniformMap::setVec2Uniform(const float &x, const float &y, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 2, x, y);}
void UniformMap::setVec3Uniform(const float &x, const float &y, const float &z, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 3, x, y, z);}
void UniformMap::setVec4Uniform(const float &x, const float &y, const float &z, const float &w, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 4, x, y, z, w);}

void UniformMap::setVec2Uniform(const vec2 &v, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 2, v.x, v.y);}
void UniformMap::setVec3Uniform(const vec3 &v, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 3, v.x, v.y, v.z);}
void UniformMap::setVec4Uniform(const vec4 &v, const std::string &name) {floatUniforms[name] = detail::FloatUniform(name, 4, v.x, v.y, v.z, v.w);}
void UniformMap::setMat4Uniform(const mat4 &m, const std::string &name) {matrixUniforms[name] = detail::MatrixUniform(name, m);}

void UniformMap::setUniformsInProgram(const ShaderProgram &program) const
{
    for (std::map<std::string, detail::IntUniform>::const_iterator i = intUniforms.begin(); i != intUniforms.end(); ++i)
    {
        const detail::IntUniform uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        
        if (location < 0)
        {
#ifndef NDEBUG
            //std::cerr << "Warning: uniform variable '" << uniform.name << "' does not exist in the GLSL program " << program.getIndex() << "!" << std::endl;
#endif
            continue;
        }
        
             if (uniform.numParameters == 1) GL_CHECK(glUniform1i(location, uniform.x));
        else if (uniform.numParameters == 2) GL_CHECK(glUniform2i(location, uniform.x, uniform.y));
        else if (uniform.numParameters == 3) GL_CHECK(glUniform3i(location, uniform.x, uniform.y, uniform.z));
        else if (uniform.numParameters == 4) GL_CHECK(glUniform4i(location, uniform.x, uniform.y, uniform.z, uniform.w));
        else std::cerr << "Warning: uniform variable '" << uniform.name << "' has an invalid number of parameters (" << uniform.numParameters << ")!" << std::endl;
    }
    
    for (std::map<std::string, detail::FloatUniform>::const_iterator i = floatUniforms.begin(); i != floatUniforms.end(); ++i)
    {
        const detail::FloatUniform uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        
        if (location < 0)
        {
#ifndef NDEBUG
            //std::cerr << "Warning: uniform variable '" << uniform.name << "' does not exist in the GLSL program " << program.getIndex() << "!" << std::endl;
#endif
            continue;
        }
        
             if (uniform.numParameters == 1) GL_CHECK(glUniform1f(location, uniform.x));
        else if (uniform.numParameters == 2) GL_CHECK(glUniform2f(location, uniform.x, uniform.y));
        else if (uniform.numParameters == 3) GL_CHECK(glUniform3f(location, uniform.x, uniform.y, uniform.z));
        else if (uniform.numParameters == 4) GL_CHECK(glUniform4f(location, uniform.x, uniform.y, uniform.z, uniform.w));
        else std::cerr << "Warning: uniform variable '" << uniform.name << "' has an invalid number of parameters (" << uniform.numParameters << ")!" << std::endl;
    }
    
    for (std::map<std::string, detail::MatrixUniform>::const_iterator i = matrixUniforms.begin(); i != matrixUniforms.end(); ++i)
    {
        const detail::MatrixUniform uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        GLfloat data[16]; 
        
        if (location < 0)
        {
#ifndef NDEBUG
            //std::cerr << "Warning: uniform matrix variable '" << uniform.name << "' does not exist in the GLSL program " << program.getIndex() << "!" << std::endl;
#endif
            continue;
        }
        
        uniform.m.toOpenGL(data);
        GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, data));
    }
}

void UniformMap::setUniformsAndTexturesInProgram(const ShaderProgram &program, const int &textureOffset) const
{
    setUniformsInProgram(program);
    
    int textureBindPoint = textureOffset;
    
    for (std::map<std::string, detail::BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        const detail::BoundTexture uniform = i->second;
        const GLint location = glGetUniformLocation(program.getIndex(), uniform.name.c_str());
        
        if (location < 0)
        {
            std::cerr << "Warning: texture '" << uniform.name << "' does not exist in the GLSL program!" << std::endl;
            continue;
        }
        
        GL_CHECK(glUniform1i(location, textureBindPoint++));
    }
}

void UniformMap::bindTextures(const int &textureOffset) const
{
    int textureBindPoint = textureOffset;
    
    for (std::map<std::string, detail::BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        if (i->second.texture) i->second.texture->bind(textureBindPoint);
        
        textureBindPoint++;
    }
}

void UniformMap::unbindTextures(const int &textureOffset) const
{
    int textureBindPoint = textureOffset;
    
    for (std::map<std::string, detail::BoundTexture>::const_iterator i = textures.begin(); i != textures.end(); ++i)
    {
        if (i->second.texture) i->second.texture->unbind(textureBindPoint);
        
        textureBindPoint++;
    }
}

