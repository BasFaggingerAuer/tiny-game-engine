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
#include <string>
#include <map>

#include <cassert>

#include <tiny/draw/glcheck.h>
#include <tiny/draw/texture.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/shader.h>
#include <tiny/draw/shaderprogram.h>
#include <tiny/draw/detail/formats.h>
#include <tiny/math/vec.h>

namespace tiny
{

namespace draw
{

namespace detail
{

template<typename T>
struct BoundUniform
{
    BoundUniform() :
        name(""),
        numParameters(0),
        x(0),
        y(0),
        z(0),
        w(0)
    {

    }
    
    BoundUniform(const std::string &a_name,
                 const size_t &a_numParameters,
                 const T &a_x,
                 const T &a_y = 0,
                 const T &a_z = 0,
                 const T &a_w = 0) :
        name(a_name),
        numParameters(a_numParameters),
        x(a_x),
        y(a_y),
        z(a_z),
        w(a_w)
    {

    }
    
    std::string name;
    size_t numParameters;
    T x, y, z, w;
};

struct MatrixUniform
{
    MatrixUniform() :
        name(""),
        m(mat4::identityMatrix())
    {

    }
    
    MatrixUniform(const std::string &a_name,
                 const mat4 &a_m) :
        name(a_name),
        m(a_m)
    {

    }
    
    std::string name;
    mat4 m;
};

typedef BoundUniform<GLfloat> FloatUniform;
typedef BoundUniform<GLint> IntUniform;

struct BoundTexture
{
    BoundTexture() :
        name(""),
        texture(0)
    {

    }
    
    BoundTexture(const std::string &a_name,
                 const TextureInterface *a_texture) :
        name(a_name),
        texture(a_texture)
    {

    }
    
    std::string name;
    const TextureInterface *texture;
};

}

class UniformMap
{
    public:
        UniformMap();
        ~UniformMap();
        
        size_t getNrUniforms() const;
        size_t getNrTextures() const;
        
        void addTexture(const std::string &name);
        void lockTextures();
        
        template <typename TextureType>
        void setTexture(const TextureType &texture, const std::string &name)
        {
            if (textures.find(name) == textures.end())
            {
                std::cerr << "Warning: texture '" << name << "' does not exist!" << std::endl;
                return;
            }
            
            textures[name].texture = &texture;
        }
        
        void setIntUniform(const int &x, const std::string &name);
        
        void setFloatUniform(const float &x, const std::string &name);
        void setVec2Uniform(const float &x, const float &y, const std::string &name);
        void setVec3Uniform(const float &x, const float &y, const float &z, const std::string &name);
        void setVec4Uniform(const float &x, const float &y, const float &z, const float &w, const std::string &name);
        
        void setVec2Uniform(const vec2 &v, const std::string &name);
        void setVec3Uniform(const vec3 &v, const std::string &name);
        void setVec4Uniform(const vec4 &v, const std::string &name);
        void setMat4Uniform(const mat4 &m, const std::string &name);
        
        void setUniformsInProgram(const ShaderProgram &) const;
        void setUniformsAndTexturesInProgram(const ShaderProgram &, const int & = 0) const;
        void bindTextures(const int & = 0) const;
        void unbindTextures(const int & = 0) const;
        
    private:
        bool texturesAreLocked;
        
        std::map<std::string, detail::IntUniform> intUniforms;
        std::map<std::string, detail::FloatUniform> floatUniforms;
        std::map<std::string, detail::MatrixUniform> matrixUniforms;
        std::map<std::string, detail::BoundTexture> textures;
};

}

}

