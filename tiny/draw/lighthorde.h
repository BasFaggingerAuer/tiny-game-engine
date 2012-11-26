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

#include <tiny/math/vec.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/icontexture2d.h>

namespace tiny
{

namespace draw
{

struct PointLightInstance
{
    PointLightInstance()
    {

    }
    
    PointLightInstance(const vec4 &a_positionAndSize,
                       const vec4 &a_colour) :
        positionAndSize(a_positionAndSize),
        colour(a_colour)
    {

    }
    
    vec4 positionAndSize;
    vec4 colour;
};

class PointLightVertexBufferInterpreter : public VertexBufferInterpreter
{
    public:
        PointLightVertexBufferInterpreter(const size_t &);
        ~PointLightVertexBufferInterpreter();
        
        VertexBuffer<PointLightInstance> instances;
};

class PointLightHorde : public Renderable
{
    public:
        PointLightHorde(const size_t &);
        ~PointLightHorde();
        
        std::string getVertexShaderCode() const;
        std::string getGeometryShaderCode() const;
        std::string getFragmentShaderCode() const;
        
        template <typename Iterator>
        void setLights(Iterator first, Iterator last)
        {
            nrLights = 0;
            
            for (Iterator i = first; i != last; ++i)
            {
                lights.instances[nrLights++] = *i;
            }
            
            lights.instances.sendToDevice();
        }
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const size_t maxNrLights;
        size_t nrLights;
        PointLightVertexBufferInterpreter lights;
};

}

}

