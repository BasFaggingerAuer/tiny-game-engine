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

#include <tiny/draw/texture.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/shader.h>
#include <tiny/draw/shaderprogram.h>
#include <tiny/draw/detail/formats.h>

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

/*! \p Renderable : an object which contains vertex, geometry and fragment shaders, textures, and vertex data required to draw an entity.
 */
class Renderable
{
    public:
        Renderable();
        Renderable(const Renderable &a_renderable);
        virtual ~Renderable();
        
        virtual std::string getVertexShaderCode() const = 0;
        virtual std::string getGeometryShaderCode() const;
        virtual std::string getFragmentShaderCode() const = 0;
        
        template <typename TextureType>
        void setTexture(const TextureType &texture, const std::string &name)
        {
            if (textures.find(name) == textures.end())
            {
                std::cerr << "Warning: texture '" << name << "' does not exist in this renderable!" << std::endl;
                return;
            }
            
            textures[name].texture = &texture;
        }
        
        void setFloatVariable(const float &x, const std::string &name);
        void setVec2Variable(const float &x, const float &y, const std::string &name);
        void setVec3Variable(const float &x, const float &y, const float &z, const std::string &name);
        void setVec4Variable(const float &x, const float &y, const float &z, const float &w, const std::string &name);
        
    protected:
        friend class Renderer;
        
        virtual void render(const ShaderProgram &) const = 0;
        void addTexture(const std::string &name);
        void setVariablesInProgram(const ShaderProgram &program) const;
        
        void renderRangeAsTriangleStrip(const size_t &first, const size_t &last) const
        {
            if (last > first) glDrawArrays(GL_TRIANGLE_STRIP, 0, last - first);
        }
        
        template <typename T>
        void renderIndicesAsTriangles(const IndexBuffer<T> &buffer) const
        {
            buffer.bind();
            glDrawElements(GL_TRIANGLES, buffer.size(), detail::getOpenGLDataType<T>(), 0);
            buffer.unbind();
        }
        
    private:
        void bindTextures() const;
        void unbindTextures() const;
        
        std::map<std::string, detail::FloatUniform> floatUniforms;
        std::map<std::string, detail::BoundTexture> textures;
};

}

}

