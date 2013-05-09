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

#include <tiny/draw/computetexture.h>

namespace tiny
{

namespace draw
{

template<typename TextureType1, typename TextureType2>
void computeResizedTexture(const TextureType1 &source, TextureType2 &dest, const vec4 &scale, const vec4 &add)
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    const std::string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec4 scaleVec;\n"
"uniform vec4 addVec;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   colour = texture(source, tex*scaleVec + addVec);\n"
"}\n";
    
    inputTextures.push_back("source");
    outputTextures.push_back("dest");

    ComputeTexture *computeTexture = new ComputeTexture(inputTextures, outputTextures, fragmentShader);
    
    computeTexture->uniformMap().setVec4Uniform(scale, "scaleVec");
    computeTexture->uniformMap().setVec4Uniform(add, "addVec");
    computeTexture->setInput(source, "source");
    computeTexture->setOutput(dest, "colour");
    computeTexture->compute();
    
    delete computeTexture;
}

}

}

