/*
Copyright 2012-2015, Bas Fagginger Auer and Matthijs van Dorp.

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
#include <list>
#include <map>
#include <set>

#include <cassert>

#include <tiny/draw/glcheck.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/shader.h>
#include <tiny/draw/shaderprogram.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/uniformmap.h>

#ifndef NDEBUG
#define RENDERER_PERFMON
#endif

namespace tiny
{

namespace draw
{

enum class CullMode
{
    CullBack,
    CullFront,
    CullNothing
};

enum class BlendMode
{
    BlendReplace,
    BlendAdd,
    BlendMix
};

namespace detail
{

class BoundProgram
{
    public:
        BoundProgram(const std::string &, const std::string &, const std::string &);
        ~BoundProgram();
        
        void bind() const;
        void unbind() const;
        void compile();
        void link();
        void bindRenderTarget(const unsigned int &, const std::string &) const;
        void setUniforms(const UniformMap &) const;
        void setUniformsAndTextures(const UniformMap &, const int & = 0) const;
        bool validate() const;
        const ShaderProgram &getProgram() const;
        
        const unsigned int hash;
        
        void addRenderableIndex(const unsigned int &);
        void freeRenderableIndex(const unsigned int &);
        unsigned int numRenderables() const;

    private:
        VertexShader *vertexShader;
        GeometryShader *geometryShader;
        FragmentShader *fragmentShader;
        ShaderProgram *program;
        
        const std::string vertexShaderCode;
        const std::string geometryShaderCode;
        const std::string fragmentShaderCode;

        std::set<unsigned int> renderableIndices;
};

class BoundRenderable
{
    public:
        BoundRenderable(Renderable *, const unsigned int &, const bool &, const bool &, const BlendMode &, const CullMode &);
        BoundRenderable(const BoundRenderable &) = delete;
        BoundRenderable & operator = (const BoundRenderable &) = delete;
        ~BoundRenderable();
    
        Renderable *renderable;
        unsigned int shaderProgramHash;
        bool readFromDepthTexture;
        bool writeToDepthTexture;
        BlendMode blendMode;
        CullMode cullMode;

#ifdef RENDERER_PERFMON
        GLuint timeQuery;
#endif
};

}

/*! \p Renderer : a class capable of drawing \p Renderable objects to \p RenderTarget objects.
 */
class Renderer
{
    public:
        Renderer(const bool &, const int &, const int &);
        virtual ~Renderer();
        
        void addRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendMode::BlendReplace, const CullMode & = CullMode::CullBack);
        bool freeRenderable(const unsigned int &);
        
        void setDepthTextureTarget(const DepthTexture2D &texture)
        {
            std::cerr << "Binding texture " << texture.getIndex() << " as depth rendering target for frame buffer " << frameBufferIndex << "." << std::endl;
            depthTargetTexture = texture.getIndex();
            
            if (static_cast<int>(texture.getWidth()) != viewportSize.x || static_cast<int>(texture.getHeight()) != viewportSize.y)
            {
                std::cerr << "Warning: Viewport size " << viewportSize.x << "x" << viewportSize.y << " is not equal to depth target texture size " << texture.getWidth() << "x" << texture.getHeight() << "!" << std::endl;
                assert(false);
            }
            
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
                    
                    if (static_cast<int>(texture.getWidth()) != viewportSize.x || static_cast<int>(texture.getHeight()) != viewportSize.y)
                    {
                        std::cerr << "Warning: Viewport size " << viewportSize.x << "x" << viewportSize.y << " is not equal to render target texture size " << texture.getWidth() << "x" << texture.getHeight() << "!" << std::endl;
                        assert(false);
                    }
                    
                    updateRenderTargets();
                    return;
                }
            }
            
            std::cerr << "Warning: render target '" << name << "' does not exist for this renderer!" << std::endl;
        }
        
        void clearTargets() const;
        std::vector<uint64_t> render() const;
        void setViewportSize(const int &, const int &);
        
    protected:
        void addRenderTarget(const std::string &name);
        
        UniformMap uniformMap;

    private:
        void createFrameBuffer();
        void destroyFrameBuffer();
        void updateRenderTargets();
        
        std::map<unsigned int, detail::BoundRenderable *> renderables;
        std::map<unsigned int, detail::BoundProgram *> shaderPrograms;
        GLuint frameBufferIndex;
        bool renderToDefaultFrameBuffer;
        std::vector<std::string> renderTargetNames;
        std::vector<GLuint> renderTargetTextures;
        GLuint depthTargetTexture;
        ivec2 viewportSize;
};

}

}

