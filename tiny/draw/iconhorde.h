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

#include <tiny/math/vec.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/colour.h>

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

class ScreenIconVertexBufferInterpreter : public VertexBufferInterpreter<ScreenIconInstance>
{
    public:
        ScreenIconVertexBufferInterpreter(const size_t &);
        ~ScreenIconVertexBufferInterpreter();
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
                icons[nrIcons++] = *i;
            }
            
            icons.sendToDevice();
        }

        /** Append text by adding text starting at a given position. This function
          * adjusts the position given such that additional text can be appended. */
        void appendText(vec4 &, const float &, const float &, const std::string &, const IconTexture2D &, const Colour &, const vec4 &);

        /** Erase the text currently in the ScreenIconHorde object. */
        void eraseText(void);
        
        void setText(const float &, const float &, const float &, const float &, const std::string &, const IconTexture2D &);

        size_t maxNumIcons(void) const { return maxNrIcons; }
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const size_t maxNrIcons;
        size_t nrIcons;
        ScreenIconVertexBufferInterpreter icons;
};

struct WorldIconInstance
{
    WorldIconInstance()
    {

    }
    
    WorldIconInstance(const vec4 &a_position,
                      const vec2 &a_size,
                      const vec4 &a_icon,
                      const vec4 &a_colour) :
        position(a_position),
        size(a_size),
        icon(a_icon),
        colour(a_colour)
    {

    }
    
    vec4 position;
    vec2 size;
    vec4 icon;
    vec4 colour;
};

class WorldIconVertexBufferInterpreter : public VertexBufferInterpreter<WorldIconInstance>
{
    public:
        WorldIconVertexBufferInterpreter(const size_t &);
        ~WorldIconVertexBufferInterpreter();
};

class WorldIconHorde : public Renderable
{
    public:
        WorldIconHorde(const size_t &, const bool &);
        ~WorldIconHorde();
        
        std::string getVertexShaderCode() const;
        std::string getGeometryShaderCode() const;
        std::string getFragmentShaderCode() const;
        
        template <typename TextureType>
        void setIconTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "iconTexture");
        }

        // Nonstandard method to set instances, preferably use setInstances().
        template <typename Iterator>
        void setIcons(Iterator first, Iterator last) { setInstances(first,last); }

        // Use same interface as StaticMeshHorde and AnimatedMeshHorde.
        template <typename Iterator>
        void setInstances(Iterator first, Iterator last)
        {
            nrIcons = 0;
            
            for (Iterator i = first; i != last && nrIcons < maxNrIcons; ++i)
            {
                icons[nrIcons++] = *i;
            }
            
            icons.sendToDevice();
        }

        
        void setText(const float &, const float &, const float &, const std::string &, const IconTexture2D &);
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const bool fullIconRotation;
        const size_t maxNrIcons;
        size_t nrIcons;
        WorldIconVertexBufferInterpreter icons;
};

}

}

