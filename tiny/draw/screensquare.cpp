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
#include <algorithm>
#include <sstream>

#include <tiny/draw/screensquare.h>

using namespace tiny::draw;

ScreenFillingSquareVertexBuffer::ScreenFillingSquareVertexBuffer() :
    VertexBuffer<vec2>(4)
{
    hostData[0] = vec2(-1.0f, 1.0f);
    hostData[1] = vec2(-1.0f,-1.0f);
    hostData[2] = vec2( 1.0f, 1.0f);
    hostData[3] = vec2( 1.0f,-1.0f);
    
    sendToDevice();
}

ScreenFillingSquareVertexBuffer::~ScreenFillingSquareVertexBuffer()
{

}

ScreenFillingSquareVertexBufferInterpreter::ScreenFillingSquareVertexBufferInterpreter() :
    VertexBufferInterpreter(),
    vertices()
{
    addVec2Attribute(vertices, 0*sizeof(float), "vertex");
}

ScreenFillingSquareVertexBufferInterpreter::~ScreenFillingSquareVertexBufferInterpreter()
{
    
}

ScreenFillingSquare::ScreenFillingSquare() :
    Renderable()
{
    
}

ScreenFillingSquare::~ScreenFillingSquare()
{

}

std::string ScreenFillingSquare::getVertexShaderCode() const
{
    return 
"#version 150\n"
"\n"
"in vec2 vertex;\n"
"\n"
"void main(void)\n"
"{\n"
"	gl_Position = vec4(vertex.x, vertex.y, 0.0f, 1.0f);\n"
"}\n\0";
}

void ScreenFillingSquare::render(const ShaderProgram &program) const
{
    //Draw screen-filling quad.
    square.bind(program);
    renderRangeAsTriangleStrip(0, 4);
    square.unbind(program);
}

