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

#include <tiny/draw/renderer.h>

using namespace tiny::draw;

Renderer::Renderer() :
    frameBufferIndex(0),
    renderTargetNames(),
    renderTargetTextures(),
    depthTargetTexture(0)
{
    
}

Renderer::Renderer(const Renderer &) :
    frameBufferIndex(0),
    renderTargetNames(),
    renderTargetTextures(),
    depthTargetTexture(0)
{
    
}

Renderer::~Renderer()
{
    //Free allocated renderable programs.
    for (std::list<detail::BoundRenderable>::iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        delete i->vertexShader;
        if (i->geometryShader) delete i->geometryShader;
        delete i->fragmentShader;
        delete i->program;
    }
    
    destroyFrameBuffer();
}

void Renderer::createFrameBuffer()
{
    //Do not create the frame buffer if it already exists.
    if (frameBufferIndex != 0) return;
    
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

void Renderer::addRenderable(Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode)
{
    assert(renderable);
    
    //Create shaders.
    VertexShader *vertexShader = new VertexShader();
    GeometryShader *geometryShader = new GeometryShader();
    FragmentShader *fragmentShader = new FragmentShader();
    ShaderProgram *program = new ShaderProgram();
    
    //Compile shaders.
    vertexShader->compile(renderable->getVertexShaderCode());
    
    if (renderable->getGeometryShaderCode().empty())
    {
        //If the renderable has no geometry shader, remove it.
        delete geometryShader;
        
        geometryShader = 0;
    }
    else
    {
        geometryShader->compile(renderable->getGeometryShaderCode());
    }
    
    fragmentShader->compile(renderable->getFragmentShaderCode());
    
    //Attach shaders.
    program->attach(*vertexShader);
    if (geometryShader) program->attach(*geometryShader);
    program->attach(*fragmentShader);
    
    //Set program outputs.
    for (size_t i = 0; i < renderTargetNames.size(); ++i)
    {
        std::cerr << "Bound '" << renderTargetNames[i].c_str() << "' to colour number " << i << " for program " << program->getIndex() << "." << std::endl;
        GL_CHECK(glBindFragDataLocation(program->getIndex(), i, renderTargetNames[i].c_str()));
    }
    
    program->link();
    
    //Lock texture uniforms to prevent the bindings from changing.
    uniformMap.lockTextures();
    renderable->uniformMap.lockTextures();
    
    //Bind uniforms and textures to program.
    program->bind();
    uniformMap.setUniformsAndTexturesInProgram(*program);
    renderable->uniformMap.setUniformsAndTexturesInProgram(*program, uniformMap.getNrTextures());
    program->unbind();
    
    renderables.push_back(detail::BoundRenderable(renderable, vertexShader, geometryShader, fragmentShader, program, readFromDepthTexture, writeToDepthTexture, blendMode));
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
}

void Renderer::updateRenderTargets()
{
    if (frameBufferIndex == 0)
        createFrameBuffer();
    
    std::vector<GLenum> drawBuffers;
    
    drawBuffers.reserve(renderTargetTextures.size() + 1);
    
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex));
    
    for (size_t i = 0; i < renderTargetTextures.size(); ++i)
    {
        if (renderTargetTextures[i] != 0)
        {
            GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, renderTargetTextures[i], 0));
            drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
        }
    }
    
    GL_CHECK(glDrawBuffers(drawBuffers.size(), &drawBuffers[0]));
    
    if (depthTargetTexture != 0)
    {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTargetTexture, 0));
    }
    
#ifndef NDEBUG    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "Warning: frame buffer " << frameBufferIndex << " is incomplete: " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << " (incomplete = " << GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT << ", wrong dimensions = " << GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT << ", missing attachment = " << GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT << ", unsupported = " << GL_FRAMEBUFFER_UNSUPPORTED << ")." << std::endl;
        assert(false);
    }
#endif
    
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Renderer::clearTargets() const
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex));
    GL_CHECK(glDepthMask(GL_TRUE));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void Renderer::render() const
{
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex));
    
    uniformMap.bindTextures();
    
    for (std::list<detail::BoundRenderable>::const_iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        if (i->readFromDepthTexture) GL_CHECK(glEnable(GL_DEPTH_TEST));
        else GL_CHECK(glDisable(GL_DEPTH_TEST));
        
        GL_CHECK(glDepthMask(i->writeToDepthTexture ? GL_TRUE : GL_FALSE));

        if (i->blendMode == BlendReplace)
        {
            GL_CHECK(glDisable(GL_BLEND));
        }
        else
        {
            GL_CHECK(glEnable(GL_BLEND));
            
                 if (i->blendMode == BlendAdd) GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
            else if (i->blendMode == BlendMix) GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        }
        
        //TODO: Is this very inefficient? Should we let the rendererable decide whether or not to update the uniforms every frame?
        i->program->bind();
        uniformMap.setUniformsInProgram(*i->program);
        i->renderable->uniformMap.setUniformsInProgram(*i->program);
        
        i->renderable->uniformMap.bindTextures(uniformMap.getNrTextures());
        i->renderable->render(*i->program);
        i->renderable->uniformMap.unbindTextures(uniformMap.getNrTextures());
    }
    
    GL_CHECK(glUseProgram(0));
    
    uniformMap.unbindTextures();
    
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

