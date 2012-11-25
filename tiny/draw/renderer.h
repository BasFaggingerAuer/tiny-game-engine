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

#include <tiny/draw/glcheck.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/shader.h>
#include <tiny/draw/shaderprogram.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/uniformmap.h>

namespace tiny
{

namespace draw
{

enum BlendMode
{
    BlendReplace,
    BlendAdd,
    BlendMix
};

namespace detail
{

struct BoundRenderable
{
    BoundRenderable(Renderable *a_renderable,
                    VertexShader *a_vertexShader,
                    GeometryShader *a_geometryShader,
                    FragmentShader *a_fragmentShader,
                    ShaderProgram *a_program,
                    const BlendMode &a_blendMode) :
        renderable(a_renderable),
        vertexShader(a_vertexShader),
        geometryShader(a_geometryShader),
        fragmentShader(a_fragmentShader),
        program(a_program),
        blendMode(a_blendMode)
    {

    }
    
    Renderable *renderable;
    VertexShader *vertexShader;
    GeometryShader *geometryShader;
    FragmentShader *fragmentShader;
    ShaderProgram *program;
    BlendMode blendMode;
};

}

/*! \p Renderer : a class capable of drawing \p Renderable objects to \p RenderTarget objects.
 */
class Renderer
{
    public:
        Renderer(const bool &, const bool &);
        virtual ~Renderer();
        
        void addRenderable(Renderable *, const BlendMode & = BlendReplace);
        
        void setDepthTextureTarget(const DepthTexture2D &texture)
        {
            std::cerr << "Binding texture " << texture.getIndex() << " as depth rendering target for frame buffer " << frameBufferIndex << "." << std::endl;
            depthTargetTexture = texture.getIndex();
            updateRenderTargets();
        }
        
        template<typename T, size_t Channels>
        void setTextureTarget(const Texture2D<T, Channels> &texture, const std::string &name)
        {
            assert(renderTargetTextures.size() == renderTargetNames.size());
            
            for (size_t i = 0; i < renderTargetNames.size(); ++i)
            {
                if (renderTargetNames[i] == name)
                {
                    std::cerr << "Binding texture " << texture.getIndex() << " as rendering target '" << name << "' for frame buffer " << frameBufferIndex << "." << std::endl;
                    renderTargetTextures[i] = texture.getIndex();
                    updateRenderTargets();
                    return;
                }
            }
            
            std::cerr << "Warning: render target '" << name << "' does not exist for this renderer!" << std::endl;
        }
        
        void clearTargets() const;
        void render() const;
        
    protected:
        void addRenderTarget(const std::string &name);
        
        UniformMap uniformMap;

    private:
        void createFrameBuffer();
        void destroyFrameBuffer();
        void updateRenderTargets();
        
        //This class should not be copied.
        Renderer(const Renderer &renderer);
        
        std::list<detail::BoundRenderable> renderables;
        GLuint frameBufferIndex;
        std::vector<std::string> renderTargetNames;
        std::vector<GLuint> renderTargetTextures;
        GLuint depthTargetTexture;
        const bool readFromDepthMap;
        const bool writeToDepthMap;
};

}

}

