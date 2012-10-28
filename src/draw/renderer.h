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

#include <exception>
#include <list>

#include <cassert>

#include <draw/renderable.h>
#include <draw/target.h>
#include <draw/shader.h>
#include <draw/shaderprogram.h>

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
                    FragmentShader *a_FragmentShader,
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
        
        void addRenderable(const Renderable *renderable);
        
        void renderToTarget(RenderTarget *target) const;
        
    protected:
        virtual std::string getFragmentShaderCode() const = 0;
        
    private:
        //This class should not be copied.
        Renderer(const Renderer &renderer);
        
        FragmentShader baseFragmentShader;
        std::list<BoundRenderable> renderables;
};

}

}

