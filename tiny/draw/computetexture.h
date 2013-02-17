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
#include <vector>
#include <string>
#include <map>

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/texture.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/renderer.h>

namespace tiny
{

namespace draw
{

struct ScreenVertex
{
    ScreenVertex()
    {

    };
    
    ScreenVertex(const vec2 &a_pos, const vec2 &a_tex) :
        pos(a_pos),
        tex(a_tex)
    {

    };
    
    vec2 pos;
    vec2 tex;
};

class SquareVertexBufferInterpreter : public VertexBufferInterpreter<ScreenVertex>
{
    public:
        SquareVertexBufferInterpreter();
        ~SquareVertexBufferInterpreter();
};

class ComputeTextureInput : public Renderable
{
    public:
        ComputeTextureInput(const std::vector<std::string> &inputNames, const std::string &a_fragmentShadercode);
        ~ComputeTextureInput();
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
        template <typename TextureType>
        void setInput(const TextureType &texture, const std::string &name)
        {
            if (inputSizes.find(name) == inputSizes.end())
            {
                std::cerr << "Warning: input texture '" << name << "' cannot be found!" << std::endl;
                return;
            }
            
            inputSizes[name] = std::pair<size_t, size_t>(texture.getWidth(), texture.getHeight());
            uniformMap.setVec2Uniform(1.0f/static_cast<float>(texture.getWidth()), 1.0f/static_cast<float>(texture.getHeight()), name + "InverseSize");
            uniformMap.setTexture(texture, name);
        }
    
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        std::map<std::string, std::pair<size_t, size_t> > inputSizes;
        const std::string fragmentShaderCode;
        SquareVertexBufferInterpreter square;
};

class ComputeTextureOutput : public Renderer
{
    public:
        ComputeTextureOutput(const std::vector<std::string> &outputNames);
        ~ComputeTextureOutput();
        
        template <typename TextureType>
        void setOutput(const TextureType &texture, const std::string &name)
        {
            setTextureTarget(texture, name);
        }
};

class ComputeTexture
{
    public:
        ComputeTexture(const std::vector<std::string> &inputNames, const std::vector<std::string> &outputNames, const std::string &fragmentShaderCode);
        ~ComputeTexture();
        
        template <typename TextureType>
        void setInput(const TextureType &texture, const std::string &name)
        {
            std::cerr << "Binding texture " << texture.getIndex() << " as input '" << name << "' for the compute shader." << std::endl;
            input.setInput(texture, name);
        }

        template <typename TextureType>
        void setOutput(const TextureType &texture, const std::string &name)
        {
            std::cerr << "Binding texture " << texture.getIndex() << " as output '" << name << "' for the compute shader." << std::endl;
            output.setOutput(texture, name);
        }
        
        void compute() const;
        UniformMap &uniformMap();
        
    protected:
        void addInput(const std::string &);
        void addOutput(const std::string &);

    private:
        ComputeTextureInput input;
        ComputeTextureOutput output;
};

}

}

