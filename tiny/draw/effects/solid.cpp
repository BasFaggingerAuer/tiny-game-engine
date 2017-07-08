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
#include <tiny/draw/effects/solid.h>

using namespace tiny;
using namespace tiny::draw::effects;

Solid::Solid()
{
    setColour(vec4(1.0f, 1.0f, 1.0f, 0.0f));
}

Solid::~Solid()
{

}

std::string Solid::getFragmentShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform vec4 mulColour;\n"
"\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   colour = mulColour;\n"
"}\n");
}

void Solid::setColour(const vec4 &a_colour)
{
    colour = a_colour;
    uniformMap.setVec4Uniform(colour, "mulColour");
}

