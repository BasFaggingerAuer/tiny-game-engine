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
#include <tiny/draw/effects/showimage.h>

using namespace tiny::draw::effects;

ShowImage::ShowImage()
{
    uniformMap.addTexture("imageTexture");
    setAspectRatio(1.0f);
    setAlpha(1.0f);
}

ShowImage::~ShowImage()
{

}

void ShowImage::setAspectRatio(const float &aspectRatio)
{
    uniformMap.setFloatUniform(aspectRatio, "aspectRatio");
}

void ShowImage::setAlpha(const float &alphaMul)
{
    uniformMap.setFloatUniform(alphaMul, "alphaMul");
}

std::string ShowImage::getFragmentShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D imageTexture;\n"
"uniform vec2 inverseScreenSize;\n"
"uniform float aspectRatio;\n"
"uniform float alphaMul;\n"
"\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   vec2 tex = gl_FragCoord.xy*inverseScreenSize;\n"
"   \n"
"   tex.y = 1.0f - tex.y;\n"
"   tex.x = 0.5f + (tex.x - 0.5f)*aspectRatio;\n"
"   \n"
"   vec4 diffuse = texture(imageTexture, tex);\n"
"   \n"
"   colour = vec4(diffuse.xyz, alphaMul*diffuse.w);\n"
"}\n");
}

