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

Renderer::Renderer()
{
    //Compile the renderer's own part of the vertex shader.
    baseFragmentShader.compile(getFragmentShaderCode());
}

Renderer::Renderer(const Renderer &renderer)
{

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

void Renderer::addRenderable(const Renderable *renderable)
{
    assert(renderable);
    
    VertexShader *vertexShader = new VertexShader();
    GeometryShader *geometryShader = new GeometryShader();
    FragmentShader *fragmentShader = new FragmentShader();
    ShaderProgram *program = new ShaderProgram();
    
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
    
    program->attach(*vertexShader);
    if (geometryShader) program->attach(*geometryShader);
    program->attach(*fragmentShader);
    program->attach(baseFragmentShader);
    
    program->link();
    
    //TODO: Bind uniforms and textures to program.
    
    renderables.push_back(BoundRenderable(renderable, vertexShader, geometryShader, fragmentShader, program));
}

void Renderer::renderToTarget(RenderTarget *target) const
{
    target->bind();
    
    for (std::list<BoundRenderable>::const_iterator i = renderables.begin(); i != renderables.end(); ++i)
    {
        i->program->bind();
        i->renderable->render(*i->program);
        //i->program->unbind();
    }
    
    target->unbind();
}

