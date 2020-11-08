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
#include <algorithm>

#include <tiny/hash/md5.h>
#include <tiny/draw/renderer.h>

using namespace tiny::draw;

detail::BoundProgram::BoundProgram(const std::string &a_vertexShaderCode, const std::string &a_geometryShaderCode, const std::string &a_fragmentShaderCode) :
    hash(tiny::hash::md5hash(a_vertexShaderCode + a_geometryShaderCode + a_fragmentShaderCode)),
    vertexShader(0),
    geometryShader(0),
    fragmentShader(0),
    program(0),
    vertexShaderCode(a_vertexShaderCode),
    geometryShaderCode(a_geometryShaderCode),
    fragmentShaderCode(a_fragmentShaderCode),
    renderableIndices()
{

}

detail::BoundProgram::~BoundProgram()
{
    if (program) delete program;
    if (vertexShader) delete vertexShader;
    if (geometryShader) delete geometryShader;
    if (fragmentShader) delete fragmentShader;
}

void detail::BoundProgram::bind() const
{
    assert(program);
    program->bind();
}

void detail::BoundProgram::unbind() const
{
    assert(program);
    program->unbind();
}

void detail::BoundProgram::bindRenderTarget(const unsigned int &index, const std::string &name) const
{
    assert(program);
    GL_CHECK(glBindFragDataLocation(program->getIndex(), index, name.c_str()));
}

void detail::BoundProgram::setUniforms(const UniformMap &map) const
{
    assert(program);
    map.setUniformsInProgram(*program);
}

void detail::BoundProgram::setUniformsAndTextures(const UniformMap &map, const int &textureOffset) const
{
    assert(program);
    map.setUniformsAndTexturesInProgram(*program, textureOffset);
}

const ShaderProgram &detail::BoundProgram::getProgram() const
{
    assert(program);
    return *program;
}

void detail::BoundProgram::compile()
{
    //Create shaders.
    if (program) delete program;
    if (vertexShader) delete vertexShader;
    if (geometryShader) delete geometryShader;
    if (fragmentShader) delete fragmentShader;
    
    vertexShader = new VertexShader();
    geometryShader = new GeometryShader();
    fragmentShader = new FragmentShader();
    program = new ShaderProgram();
    
    //Compile shaders.
    vertexShader->compile(vertexShaderCode);
    
    if (geometryShaderCode.empty())
    {
        //If the renderable has no geometry shader, remove it.
        delete geometryShader;
        
        geometryShader = 0;
    }
    else
    {
        geometryShader->compile(geometryShaderCode);
    }
    
    fragmentShader->compile(fragmentShaderCode);
    
    //Attach shaders.
    program->attach(*vertexShader);
    if (geometryShader) program->attach(*geometryShader);
    program->attach(*fragmentShader);
}

void detail::BoundProgram::link()
{
    assert(program);
    program->link();
}

bool detail::BoundProgram::validate() const
{
    if (program) return program->validate();
    
    return false;
}

void detail::BoundProgram::addRenderableIndex(const unsigned int &renderableIndex)
{
    assert(renderableIndices.count(renderableIndex) == 0);
    
    renderableIndices.insert(renderableIndex);
}

void detail::BoundProgram::freeRenderableIndex(const unsigned int &renderableIndex)
{
    renderableIndices.erase(renderableIndex);
}

unsigned int detail::BoundProgram::numRenderables() const
{
    return renderableIndices.size();
}

detail::BoundRenderable::BoundRenderable(Renderable *a_renderable,
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
#ifdef RENDERER_PERFMON
    GL_CHECK(glGenQueries(1, &timeQuery));
#endif
}

detail::BoundRenderable::~BoundRenderable()
{
#ifdef RENDERER_PERFMON
    if (timeQuery != 0) GL_CHECK(glDeleteQueries(1, &timeQuery));
#endif
}

Renderer::Renderer(const bool & a_renderToDefaultFrameBuffer, const int &a_viewportWidth, const int &a_viewportHeight) :
    frameBufferIndex(0),
    renderToDefaultFrameBuffer(a_renderToDefaultFrameBuffer),
    renderTargetNames(),
    renderTargetTextures(),
    depthTargetTexture(0),
    viewportSize(a_viewportWidth, a_viewportHeight)
{
    
}

Renderer::~Renderer()
{
    //Free allocated bound renderables.
    for (auto i = renderables.begin(); i != renderables.end(); ++i)
    {
        delete i->second;
    }
    
    //Free allocated renderable programs.
    for (auto i = shaderPrograms.begin(); i != shaderPrograms.end(); ++i)
    {
        delete i->second;
    }
    
    destroyFrameBuffer();
}

void Renderer::setViewportSize(const int &a_viewportWidth, const int &a_viewportHeight)
{
    if (renderToDefaultFrameBuffer)
    {
        viewportSize = ivec2(a_viewportWidth, a_viewportHeight);
    }
    else
    {
        std::cerr << "Warning: It is not permitted to adjust the viewport size of a non-default target renderer!" << std::endl;
        assert(false);
    }
}

void Renderer::createFrameBuffer()
{
    //Do not create the frame buffer if it already exists.
    if (frameBufferIndex != 0 || renderToDefaultFrameBuffer) return;
    
    GL_CHECK(glGenFramebuffers(1, &frameBufferIndex));
    
    if (frameBufferIndex == 0)
        throw std::bad_alloc();
}

void Renderer::destroyFrameBuffer()
{
    renderTargetNames.clear();
    renderTargetTextures.clear();
    
    if (frameBufferIndex != 0)
        GL_CHECK(glDeleteFramebuffers(1, &frameBufferIndex));
    
    frameBufferIndex = 0;
}

void Renderer::addRenderable(const unsigned int &renderableIndex, Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode, const CullMode &cullMode)
{
    assert(renderable);
    
    //Verify that this index is not yet in use.
    std::map<unsigned int, detail::BoundRenderable *>::iterator j = renderables.find(renderableIndex);
    
    if (j != renderables.end())
    {
        std::cerr << "A renderable with index " << renderableIndex << " has already been added to the renderer!" << std::endl;
        throw std::exception();
    }
    
    detail::BoundProgram *shaderProgram = new detail::BoundProgram(renderable->getVertexShaderCode(), renderable->getGeometryShaderCode(), renderable->getFragmentShaderCode());
    std::map<unsigned int, detail::BoundProgram *>::iterator k = shaderPrograms.find(shaderProgram->hash);

    //Lock texture uniforms to prevent the bindings from changing.
    uniformMap.lockTextures();
    renderable->uniformMap.lockTextures();
        
    //Has this program already been compiled earlier?
    if (k == shaderPrograms.end())
    {
        //If not, then add it to the list of programs.
        shaderProgram->compile();
        
        //Set program outputs.
        for (size_t i = 0; i < renderTargetNames.size(); ++i)
        {
            std::cerr << "Bound '" << renderTargetNames[i].c_str() << "' to colour number " << i << " in frame buffer " << frameBufferIndex << " for program " << shaderProgram->getProgram().getIndex() << "." << std::endl;
            shaderProgram->bindRenderTarget(i, renderTargetNames[i]);
        }
        
        shaderProgram->link();
        
        //Bind uniforms and textures to program.
        shaderProgram->bind();
        shaderProgram->setUniformsAndTextures(uniformMap);
        shaderProgram->setUniformsAndTextures(renderable->uniformMap, uniformMap.getNrTextures());
        shaderProgram->unbind();
    
#ifndef NDEBUG
        if (!shaderProgram->validate())
        {
            std::cerr << "Unable to validate program!" << std::endl;
            throw std::exception();
        }
#endif
        
        k = shaderPrograms.insert(std::make_pair(shaderProgram->hash, shaderProgram)).first;
    }
    else
    {
        delete shaderProgram;
    }
    
    k->second->addRenderableIndex(renderableIndex);
    renderables.insert(std::make_pair(renderableIndex, new detail::BoundRenderable(renderable, k->first, readFromDepthTexture, writeToDepthTexture, blendMode, cullMode)));
}

bool Renderer::freeRenderable(const unsigned int &renderableIndex)
{
    std::map<unsigned int, detail::BoundRenderable *>::iterator j = renderables.find(renderableIndex);
    
    if (j == renderables.end())
    {
        std::cerr << "Unable to find renderable with index " << renderableIndex << "!" << std::endl;
        return false;
    }
    
    detail::BoundRenderable *renderable = j->second;
    std::map<unsigned int, detail::BoundProgram *>::iterator k = shaderPrograms.find(renderable->shaderProgramHash);
    
    assert(k != shaderPrograms.end());
    k->second->freeRenderableIndex(renderableIndex);
    
    //Remove renderable.
    delete renderable;
    renderables.erase(j);
    
    //Remove shader program if it is no longer referenced by any renderable.
    if (k->second->numRenderables() == 0)
    {
        delete k->second;
        shaderPrograms.erase(k);
    }

    return true;
}

void Renderer::addRenderTarget(const std::string &name)
{
    if (std::find(renderTargetNames.begin(), renderTargetNames.end(), name) != renderTargetNames.end())
    {
        std::cerr << "Warning: render target '" << name << "' already exists!" << std::endl;
        return;
    }
    
    renderTargetNames.push_back(name);
    renderTargetTextures.push_back(0);
    
    if (renderTargetNames.size() >= GL_MAX_DRAW_BUFFERS)
    {
        std::cerr << "Warning: binding more than the maximum number (" << GL_MAX_DRAW_BUFFERS << ") of draw buffers!" << std::endl;
    }

    if (renderToDefaultFrameBuffer && renderTargetNames.size() > 1)
    {
        std::cerr << "Warning: binding multiple outputs to default frame buffer!" << std::endl;
        assert(false);
    }

    updateRenderTargets();
}

void Renderer::updateRenderTargets()
{
    if (frameBufferIndex == 0 && !renderToDefaultFrameBuffer)
        createFrameBuffer();
    
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIndex));
    
    if (!renderToDefaultFrameBuffer)
    {
        std::vector<GLenum> drawBuffers;
        
        drawBuffers.reserve(renderTargetTextures.size() + 1);

        for (size_t i = 0; i < renderTargetTextures.size(); ++i)
        {
            if (renderTargetTextures[i] != 0)
            {
                GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTargetTextures[i], 0));
                drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
            }
        }
    
        GL_CHECK(glDrawBuffers(drawBuffers.size(), &drawBuffers[0]));
    }
    else
    {
        //Draw directly to the screen backbuffer.
        assert(frameBufferIndex == 0);
        assert(renderTargetNames.size() == 1);
        assert(renderTargetTextures.size() == 1);
        assert(renderTargetTextures[0] == 0);
        
        GL_CHECK(glDrawBuffer(GL_BACK));
    }
    
    if (depthTargetTexture != 0)
    {
        GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTargetTexture, 0));
    }
    
#ifndef NDEBUG
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Warning: frame buffer " << frameBufferIndex << " is incomplete: " << glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) << " (incomplete = " << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << ", wrong dimensions = " << GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT << ", missing attachment = " << GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT << ", unsupported = " << GL_FRAMEBUFFER_UNSUPPORTED << ")." << std::endl;
        //assert(false);
    }
#endif
    
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

void Renderer::clearTargets() const
{
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIndex));
    GL_CHECK(glDepthMask(GL_TRUE));
    GL_CHECK(glClear(depthTargetTexture != 0 ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT));
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
}

std::vector<uint64_t> Renderer::render() const
{
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferIndex));
    //GL_CHECK(glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT));
    
    if (viewportSize.x > 0 && viewportSize.y > 0)
    {
        GL_CHECK(glViewport(0, 0, viewportSize.x, viewportSize.y));
    }
    
    //Try to minimize shader program switching.
    unsigned int lastShaderProgramHash = 0;
    const detail::BoundProgram *shaderProgram = 0;
    unsigned int nrShaderProgramSwitches = 0;
    
    uniformMap.bindTextures();
    
    for (auto i = renderables.cbegin(); i != renderables.cend(); ++i)
    {
        const detail::BoundRenderable *renderable = i->second;
        
#ifdef RENDERER_PERFMON
        GL_CHECK(glBeginQuery(GL_TIME_ELAPSED, renderable->timeQuery));
#endif

        if (renderable->readFromDepthTexture) GL_CHECK(glEnable(GL_DEPTH_TEST));
        else GL_CHECK(glDisable(GL_DEPTH_TEST));
        
        GL_CHECK(glDepthMask(renderable->writeToDepthTexture ? GL_TRUE : GL_FALSE));

        if (renderable->blendMode == BlendMode::BlendReplace)
        {
            GL_CHECK(glDisable(GL_BLEND));
        }
        else
        {
            GL_CHECK(glEnable(GL_BLEND));
            
                 if (renderable->blendMode == BlendMode::BlendAdd) GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            else if (renderable->blendMode == BlendMode::BlendMix) GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
        
        if (renderable->cullMode == CullMode::CullBack)
        {
            GL_CHECK(glEnable(GL_CULL_FACE));
            GL_CHECK(glCullFace(GL_BACK));
        }
        else if (renderable->cullMode == CullMode::CullFront)
        {
            GL_CHECK(glEnable(GL_CULL_FACE));
            GL_CHECK(glCullFace(GL_FRONT));
        }
        else if (renderable->cullMode == CullMode::CullNothing)
        {
            GL_CHECK(glDisable(GL_CULL_FACE));
        }
        
        //TODO: Is this very inefficient? Should we let the rendererable decide whether or not to update the uniforms every frame?
        if (renderable->shaderProgramHash != lastShaderProgramHash)
        {
            //If we have a different shader program, bind it.
            lastShaderProgramHash = renderable->shaderProgramHash;
            ++nrShaderProgramSwitches;
            
            //We should never have invalid hashes.
            assert(shaderPrograms.find(lastShaderProgramHash) != shaderPrograms.end());
            shaderProgram = shaderPrograms.find(lastShaderProgramHash)->second;
            shaderProgram->bind();
            shaderProgram->setUniforms(uniformMap);
        }
        
        assert(shaderProgram);
        
        shaderProgram->setUniforms(renderable->renderable->uniformMap);
        
        renderable->renderable->uniformMap.bindTextures(uniformMap.getNrTextures());
        renderable->renderable->bind();
        renderable->renderable->render(shaderProgram->getProgram());
        renderable->renderable->unbind();
        renderable->renderable->uniformMap.unbindTextures(uniformMap.getNrTextures());

#ifdef RENDERER_PERFMON
        GL_CHECK(glEndQuery(GL_TIME_ELAPSED));
#endif
    }
    
    GL_CHECK(glUseProgram(0));
    
    uniformMap.unbindTextures();
    
    //GL_CHECK(glPopAttrib());
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));

    std::vector<uint64_t> renderTimesInNs;

#ifdef RENDERER_PERFMON
    //Do performance querying after all rendering is complete to allow asynchronous queries.
    renderTimesInNs.reserve(renderables.size());

    for (auto i = renderables.cbegin(); i != renderables.cend(); ++i)
    {
        const detail::BoundRenderable *renderable = i->second;

        GLuint64 query = 0;

        GL_CHECK(glGetQueryObjectui64v(renderable->timeQuery, GL_QUERY_RESULT, &query));

        renderTimesInNs.push_back(static_cast<int64_t>(query));
    }
#endif
    
#ifndef NDEBUG
    //std::cerr << "Switched shaders " << nrShaderProgramSwitches << " times for " << renderables.size() << " renderables." << std::endl;
#endif

    return renderTimesInNs;
}

