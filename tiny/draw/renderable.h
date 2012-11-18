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
#include <tiny/draw/uniformmap.h>
#include <tiny/draw/detail/formats.h>

namespace tiny
{

namespace draw
{

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
        
    protected:
        friend class Renderer;
        
        virtual void render(const ShaderProgram &) const = 0;
        
        void renderRangeAsTriangleStrip(const size_t &first, const size_t &last) const
        {
            if (last > first) GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, last - first));
        }
        
        void renderRangeAsPoints(const size_t &first, const size_t &last) const
        {
            if (last > first) GL_CHECK(glDrawArrays(GL_POINTS, 0, last - first));
        }
        
        template <typename T>
        void renderIndicesAsTriangles(const IndexBuffer<T> &buffer) const
        {
            buffer.bind();
            GL_CHECK(glDrawElements(GL_TRIANGLES, buffer.size(), detail::getOpenGLDataType<T>(), 0));
            buffer.unbind();
        }
        
        UniformMap uniformMap;
};

}

}

