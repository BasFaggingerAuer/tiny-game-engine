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

#include <draw/renderer.h>

using namespace tiny::draw;

Renderer::Renderer() :
    frameBufferIndex(0),
    renderTargetNames()
{
    
}

Renderer::Renderer(const Renderer &renderer)
{
    destroyFrameBuffer();
}

Renderer::~Renderer()
{
    //Free allocated renderable programs.
    for (std::list<BoundRenderable>::iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        delete i->vertexShader;
        if (i->geometryShader) delete i->geometryShader;
        delete i->fragmentShader;
        delete i->program;
    }
}

void Renderer::createFrameBuffer()
{
    //Do not create the frame buffer if it already exists.
    if (frameBufferIndex != 0) return;
    
    glGenFrameBuffers(1, &frameBufferIndex);
    
    if (frameBufferIndex == 0)
        throw std::bad_alloc();
}

void Renderer::destroyFrameBuffer()
{
    renderTargetNames.clear();
    
    if (frameBufferIndex != 0)
        glDeleteFrameBuffers(1, &frameBufferIndex);
    
    frameBufferIndex = 0;
}

void Renderer::addRenderable(const Renderable *renderable)
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
    
    //Bind uniforms and textures to program.
    renderable->setVariablesInProgram(*program);
    
    renderables.push_back(BoundRenderable(renderable, vertexShader, geometryShader, fragmentShader, program));
}

void Renderer::render() const
{
    glBindFrameBuffer(GL_FRAME_BUFFER, frameBufferIndex);
    
    for (std::list<BoundRenderable>::const_iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        i->program->bind();
        i->bindTextures();
        i->renderable->render();
        i->unbindTextures();
        //i->program->unbind();
    }
    
    glBindFrameBuffer(GL_FRAME_BUFFER, 0);
}

