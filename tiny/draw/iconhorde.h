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

struct ScreenIconInstance
{
    ScreenIconInstance()
    {

    }
    
    ScreenIconInstance(const vec4 &a_positionAndSize,
                       const vec4 &a_icon,
                       const vec4 &a_colour) :
        positionAndSize(a_positionAndSize),
        icon(a_icon),
        colour(a_colour)
    {

    }
    
    vec4 positionAndSize;
    vec4 icon;
    vec4 colour;
};

class ScreenIconVertexBufferInterpreter : public VertexBufferInterpreter
{
    public:
        ScreenIconVertexBufferInterpreter(const size_t &);
        ~ScreenIconVertexBufferInterpreter();
        
        VertexBuffer<ScreenIconInstance> instances;
};

class ScreenIconHorde : public Renderable
{
    public:
        ScreenIconHorde(const size_t &);
        ~ScreenIconHorde();
        
        std::string getVertexShaderCode() const;
        std::string getGeometryShaderCode() const;
        std::string getFragmentShaderCode() const;
        
        template <typename TextureType>
        void setIconTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "iconTexture");
        }
        
        template <typename Iterator>
        void setIcons(Iterator first, Iterator last)
        {
            nrIcons = 0;
            
            for (Iterator i = first; i != last && nrIcons < maxNrIcons; ++i)
            {
                icons.instances[nrIcons++] = *i;
            }
            
            icons.instances.sendToDevice();
        }
        
        void setText(const float &, const float &, const float &, const float &, const std::string &, const IconTexture2D &);
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const size_t maxNrIcons;
        size_t nrIcons;
        ScreenIconVertexBufferInterpreter icons;
};

}

}

