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
void computeNormalMap(const TextureType1 &heightMap, TextureType2 &normalMap, const float &mapScale)
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    const std::string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceInverseSize;\n"
"uniform float mapScale;\n"
"\n"
"in vec2 tex;\n"
"out vec4 normal;\n"
"\n"
"void main(void)\n"
"{\n"
"   float east = texture(source, tex + vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float west = texture(source, tex - vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float north = texture(source, tex + vec2(0.0f, sourceInverseSize.y)).x;\n"
"   float south = texture(source, tex - vec2(0.0f, sourceInverseSize.y)).x;\n"
"   \n"
"   vec3 n = normalize(vec3(west - east, mapScale, south - north));\n"
"   \n"
"   normal = vec4(0.5f*(n + 1.0f), 1.0f);\n"
"}\n";
    
    inputTextures.push_back("source");
    outputTextures.push_back("normal");

    ComputeTexture *computeTexture = new ComputeTexture(inputTextures, outputTextures, fragmentShader);
    
    computeTexture->uniformMap().setFloatUniform(2.0f*mapScale, "mapScale");
    computeTexture->setInput(heightMap, "source");
    computeTexture->setOutput(normalMap, "normal");
    computeTexture->compute();
    normalMap.getFromDevice();
    
    delete computeTexture;
}

}

}

