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

#include <vector>
#include <string>
#include <exception>

#include <tiny/draw/computetexture.h>

namespace tiny
{

namespace draw
{

template<typename TextureType>
void computeDiamondSquareRefinement(const TextureType &source, TextureType &dest, const size_t &stepSize)
{
    if (stepSize >= source.getWidth() || stepSize >= source.getHeight() || stepSize <= 1 || (stepSize & (stepSize - 1)) != 0)
    {
        std::cerr << "Invalid step size " << stepSize << " for diamond square algorithm on a " << source.getWidth() << "x" << source.getHeight() << " height map. Should be a power of two." << std::endl;
        throw std::exception();
    }
    
    //Allocate temporary textures.
    TextureType *tmp[2];
    
    tmp[0] = new TextureType(source.getWidth(), source.getHeight());
    tmp[1] = new TextureType(source.getWidth(), source.getHeight());
    
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    
    inputTextures.push_back("source");
    outputTextures.push_back("dest");
    
    const std::string diamondFragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceSize;\n"
"uniform int step;\n"
"uniform vec2 sourceStep;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"float rand(vec2 co)\n"
"{\n"
"	return 0.5f*fract(sin(dot(co.xy, vec2(12.9898f,78.233f)))*43758.5453f) - 0.25f;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"	ivec2 itex = ivec2(sourceSize*tex);\n"
"	ivec2 twoStepMod = ivec2(itex.x & (2*step - 1), itex.y & (2*step - 1));\n"
"	ivec2 stepMod = ivec2(itex.x & (step - 1), itex.y & (step - 1));\n"
"	\n"
"	//Are we at a corner?\n"
"	if (twoStepMod.x == 0 && twoStepMod.y == 0)\n"
"	{\n"
"		colour = texture(source, tex);\n"
"	}\n"
"	//Are we at a diamond's vertex in a row?\n"
"	else if (twoStepMod.y == 0 && stepMod.x == 0)\n"
"	{\n"
"		float h1 = texture(source, vec2(tex.x - sourceStep.x, tex.y)).x;\n"
"		float h2 = texture(source, vec2(tex.x + sourceStep.x, tex.y)).x;\n"
"		colour = vec4(0.5f*(h1 + h2) + (0.5f + abs(h2 - h1))*rand(tex), 0.0f, 0.0f, 0.0f);\n"
"	}\n"
"	//Are we at a diamond's vertex in a column?\n"
"	else if (twoStepMod.x == 0 && stepMod.y == 0)\n"
"	{\n"
"		float h1 = texture(source, vec2(tex.x, tex.y - sourceStep.y)).x;\n"
"		float h2 = texture(source, vec2(tex.x, tex.y + sourceStep.y)).x;\n"
"		colour = vec4(0.5f*(h1 + h2) + (0.5f + abs(h2 - h1))*rand(tex), 0.0f, 0.0f, 0.0f);\n"
"	}\n"
"	//Otherwise just copy.\n"
"	else\n"
"	{\n"
"		colour = texture(source, tex);\n"
"	}\n"
"}\n";
		const std::string squareFragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceSize;\n"
"uniform int step;\n"
"uniform vec2 sourceStep;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"float rand(vec2 co)\n"
"{\n"
"	return 0.5f*fract(sin(dot(co.xy, vec2(78.233f, 12.9898f)))*43758.5453f) - 0.25f;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"	ivec2 itex = ivec2(sourceSize*tex);\n"
"	ivec2 twoStepMod = ivec2(itex.x & (2*step - 1), itex.y & (2*step - 1));\n"
"	ivec2 stepMod = ivec2(itex.x & (step - 1), itex.y & (step - 1));\n"
"	\n"
"	//Are we at a square's centre?\n"
"	if (twoStepMod.x != 0 && twoStepMod.y != 0 && stepMod.x == 0 && stepMod.y == 0)\n"
"	{\n"
"		float h1 = texture(source, vec2(tex.x - sourceStep.x, tex.y)).x;\n"
"		float h2 = texture(source, vec2(tex.x + sourceStep.x, tex.y)).x;\n"
"		float h3 = texture(source, vec2(tex.x, tex.y - sourceStep.y)).x;\n"
"		float h4 = texture(source, vec2(tex.x, tex.y + sourceStep.y)).x;\n"
"		colour = vec4(0.25f*(h1 + h2 + h3 + h4) + (0.5f + abs(max(h2 - h1, h4 - h3)))*rand(tex), 0.0f, 0.0f, 0.0f);\n"
"	}\n"
"	//Otherwise just copy.\n"
"	else\n"
"	{\n"
"		colour = texture(source, tex);\n"
"	}\n"
"}\n";
    
    ComputeTexture *diamondComputeTexture = new ComputeTexture(inputTextures, outputTextures, diamondFragmentShader);
    ComputeTexture *squareComputeTexture = new ComputeTexture(inputTextures, outputTextures, squareFragmentShader);
    
    diamondComputeTexture->uniformMap().setVec2Uniform(source.getWidth(), source.getHeight(), "sourceSize");
    diamondComputeTexture->setOutput(tmp[1], "dest");
    
    squareComputeTexture->uniformMap().setVec2Uniform(source.getWidth(), source.getHeight(), "sourceSize");
    squareComputeTexture->setInput(tmp[1], "source");
    squareComputeTexture->setOutput(tmp[0], "dest");
    
    //Run diamond-square.
    for (size_t step = stepSize/2; step >= 1; step >>= 1)
    {
        diamondComputeTexture->uniformMap().setIntUniform(step, "step");
        diamondComputeTexture->uniformMap().setVec2Uniform(static_cast<float>(step)/static_cast<float>(source.getWidth()), static_cast<float>(step)/static_cast<float>(source.getHeight()), "sourceStep");
        squareComputeTexture->uniformMap().setIntUniform(step, "step");
        squareComputeTexture->uniformMap().setVec2Uniform(static_cast<float>(step)/static_cast<float>(source.getWidth()), static_cast<float>(step)/static_cast<float>(source.getHeight()), "sourceStep");
        
        if (step == stepSize/2)
        {
            //Use given heightmap as source texture for the first step.
            diamondComputeTexture->setInput(source, "source");
        }
        else
        {
            diamondComputeTexture->setInput(tmp[0], "source");
        }
        
        if (step == 1)
        {
            //Use given output texture as target for the final step.
            squareComputeTexture->setOutput(dest, "dest");
        }
        else
        {
            squareComputeTexture->setOutput(tmp[0], "dest");
        }
        
        diamondComputeTexture->compute();
        squareComputeTexture->compute();
    }
    
    //Free all data.
    delete diamondComputeTexture;
    delete squareComputeTexture;
    delete tmp[0];
    delete tmp[1];
}

}

}

