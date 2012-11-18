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
    renderTargetNames()
{
    
}

Renderer::Renderer(const Renderer &)
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
    
    glGenFramebuffers(1, &frameBufferIndex);
    
    if (frameBufferIndex == 0)
        throw std::bad_alloc();
}

void Renderer::destroyFrameBuffer()
{
    renderTargetNames.clear();
    
    if (frameBufferIndex != 0)
        glDeleteFramebuffers(1, &frameBufferIndex);
    
    frameBufferIndex = 0;
}

void Renderer::addRenderable(Renderable *renderable)
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
        glBindFragDataLocation(program->getIndex(), i, renderTargetNames[i].c_str());
    }
    
    program->link();
    
    //Lock texture uniforms to prevent the bindings from changing.
    uniformMap.lockTextures();
    renderable->uniformMap.lockTextures();
    
    //Bind uniforms and textures to program.
    uniformMap.setUniformsAndTexturesInProgram(*program);
    renderable->uniformMap.setUniformsAndTexturesInProgram(*program, uniformMap.getNrTextures());
    
    renderables.push_back(detail::BoundRenderable(renderable, vertexShader, geometryShader, fragmentShader, program));
}

void Renderer::addRenderTarget(const std::string &name)
{
    if (std::find(renderTargetNames.begin(), renderTargetNames.end(), name) != renderTargetNames.end())
    {
        std::cerr << "Warning: render target '" << name << "' already exists!" << std::endl;
        return;
    }
    
    renderTargetNames.push_back(name);
    
    //Create a frame buffer if we render to more than a single target.
    if (renderTargetNames.size() >= 2 && frameBufferIndex == 0) createFrameBuffer();
    
    if (renderTargetNames.size() >= GL_MAX_DRAW_BUFFERS)
    {
        std::cerr << "Warning: binding more than the maximum number (" << GL_MAX_DRAW_BUFFERS << ") of draw buffers!" << std::endl;
    }
}

void Renderer::render() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferIndex);
    
    uniformMap.bindTextures();
    
    for (std::list<detail::BoundRenderable>::const_iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        //TODO: Is this very inefficient? Should we let the rendererable decide whether or not to update the uniforms every frame?
        //i->renderable->setVariablesInProgram(*i->program);
        i->program->bind();
        uniformMap.setUniformsAndTexturesInProgram(*i->program);
        i->renderable->uniformMap.bindTextures(uniformMap.getNrTextures());
        i->renderable->render(*i->program);
        i->renderable->uniformMap.unbindTextures(uniformMap.getNrTextures());
    }
    
    glUseProgram(0);
    
    uniformMap.unbindTextures();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

