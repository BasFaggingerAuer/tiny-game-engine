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
#include <list>

#include <cassert>

#include <tiny/draw/renderable.h>
#include <tiny/draw/shader.h>
#include <tiny/draw/shaderprogram.h>
#include <tiny/draw/texture2d.h>

namespace tiny
{

namespace draw
{

namespace detail
{

struct BoundRenderable
{
    BoundRenderable(Renderable *a_renderable,
                    VertexShader *a_vertexShader,
                    GeometryShader *a_geometryShader,
                    FragmentShader *a_fragmentShader,
                    ShaderProgram *a_program) :
        renderable(a_renderable),
        vertexShader(a_vertexShader),
        geometryShader(a_geometryShader),
        fragmentShader(a_fragmentShader),
        program(a_program)
    {

    }
    
    Renderable *renderable;
    VertexShader *vertexShader;
    GeometryShader *geometryShader;
    FragmentShader *fragmentShader;
    ShaderProgram *program;
};

}

/*! \p Renderer : a class capable of drawing \p Renderable objects to \p RenderTarget objects.
 */
class Renderer
{
    public:
        Renderer();
        virtual ~Renderer();
        
        void addRenderable(Renderable *renderable);
        
        template<typename T, size_t Channels>
        void setTextureTarget(const Texture2D<T, Channels> &texture, const std::string &name)
        {
            if (frameBufferIndex == 0) createFrameBuffer();
            
            for (size_t i = 0; i < renderTargetNames.size(); ++i)
            {
                if (renderTargetNames[i] == name)
                {
                    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex);
                    glFrameBufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture.getIndex(), 0);
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    return;
                }
            }
            
            std::cerr << "Warning: render target '" << name << "' does not exist for this renderer!" << std::endl;
        }
        
        void render() const;
        
    protected:
        void addRenderTarget(const std::string &name);
        
    private:
        void createFrameBuffer();
        void destroyFrameBuffer();
        
        //This class should not be copied.
        Renderer(const Renderer &renderer);
        
        std::list<detail::BoundRenderable> renderables;
        GLuint frameBufferIndex;
        std::vector<std::string> renderTargetNames;
};

}

}

