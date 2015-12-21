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

namespace tiny
{

namespace draw
{

enum CullMode
{
    CullBack,
    CullFront,
    CullNothing
};

enum BlendMode
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

struct BoundRenderable
{
    BoundRenderable(Renderable *a_renderable,
                    const unsigned int &a_shaderProgramHash,
                    const bool &a_readFromDepthTexture,
                    const bool &a_writeToDepthTexture,
                    const BlendMode &a_blendMode,
                    const CullMode &a_cullMode) :
        renderable(a_renderable),
        shaderProgramHash(a_shaderProgramHash),
        readFromDepthTexture(a_readFromDepthTexture),
        writeToDepthTexture(a_writeToDepthTexture),
        blendMode(a_blendMode),
        cullMode(a_cullMode)
    {

    }
    
    Renderable *renderable;
    unsigned int shaderProgramHash;
    bool readFromDepthTexture;
    bool writeToDepthTexture;
    BlendMode blendMode;
    CullMode cullMode;
};

}

/*! \p Renderer : a class capable of drawing \p Renderable objects to \p RenderTarget objects.
 */
class Renderer
{
    public:
        Renderer();
        virtual ~Renderer();
        
        void addRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace, const CullMode & = CullBack);
        bool freeRenderable(const unsigned int &);
        
        void setDepthTextureTarget(const DepthTexture2D &texture)
        {
            std::cerr << "Binding texture " << texture.getIndex() << " as depth rendering target for frame buffer " << frameBufferIndex << "." << std::endl;
            depthTargetTexture = texture.getIndex();
            
            if (static_cast<int>(texture.getWidth()) != viewportSize.x) viewportSize.x = texture.getWidth();
            if (static_cast<int>(texture.getHeight()) != viewportSize.y) viewportSize.y = texture.getHeight();
            
            updateRenderTargets();
        }
        
        void clearTargets() const;
        void render() const;
        
    protected:
        void addRenderTarget(const std::string &name);
        
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
                    
                    if (static_cast<int>(texture.getWidth()) != viewportSize.x) viewportSize.x = texture.getWidth();
                    if (static_cast<int>(texture.getHeight()) != viewportSize.y) viewportSize.y = texture.getHeight();
                    
                    updateRenderTargets();
                    return;
                }
            }
            
            std::cerr << "Warning: render target '" << name << "' does not exist for this renderer!" << std::endl;
        }
        
        
        UniformMap uniformMap;

    private:
        void createFrameBuffer();
        void destroyFrameBuffer();
        void updateRenderTargets();
        
        //This class should not be copied.
        Renderer(const Renderer &renderer);
        
        std::map<unsigned int, detail::BoundRenderable *> renderables;
        std::map<unsigned int, detail::BoundProgram *> shaderPrograms;
        GLuint frameBufferIndex;
        std::vector<std::string> renderTargetNames;
        std::vector<GLuint> renderTargetTextures;
        GLuint depthTargetTexture;
        ivec2 viewportSize;
};

}

}

