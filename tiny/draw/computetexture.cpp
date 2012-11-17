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

#include <tiny/draw/computetexture.h>

using namespace tiny::draw;

SquareVertexBuffer::SquareVertexBuffer() :
    VertexBuffer<float>(16)
{
    const float squareData[16] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f
        };
    
    assign(squareData, squareData + 16);
}

SquareVertexBuffer::~SquareVertexBuffer()
{

}

SquareVertexBufferInterpreter::SquareVertexBufferInterpreter() :
    VertexBufferInterpreter(),
    vertices()
{
    addVec2Attribute(vertices, 0, "vertex");
    addVec2Attribute(vertices, 2*sizeof(float), "textureCoordinates");
}

SquareVertexBufferInterpreter::~SquareVertexBufferInterpreter()
{
    
}

ComputeTextureInput::ComputeTextureInput(const std::vector<std::string> &inputNames, const std::string &a_fragmentShaderCode) :
    Renderable(),
    fragmentShaderCode(a_fragmentShaderCode)
{
    for (std::vector<std::string>::const_iterator i = inputNames.begin(); i != inputNames.end(); ++i)
    {
        addTexture(*i);
        inputSizes[*i] = std::pair<size_t, size_t>(1, 1);
    }
}

ComputeTextureInput::~ComputeTextureInput()
{

}

std::string ComputeTextureInput::getVertexShaderCode() const
{
    return 
"#version 150\n"
"\n"
"in vec2 vertex;\n"
"in vec2 textureCoordinate;\n"
"\n"
"out vec2 tex;\n"
"\n"
"void main(void)\n"
"{\n"
"	gl_Position = vec4(vertex.x, vertex.y, 0.0f, 1.0f);\n"
"	tex = textureCoordinate;\n"
"}\n\0";
}

std::string ComputeTextureInput::getFragmentShaderCode() const
{
    return fragmentShaderCode;
}

void ComputeTextureInput::render(const ShaderProgram &program) const
{
    //Update input texture sizes.
    setVariablesInProgram(program);
    
    //Draw screen-filling quad.
    square.bind(program);
    renderRangeAsTriangleStrip(0, 4);
    square.unbind(program);
}

ComputeTextureOutput::ComputeTextureOutput(const std::vector<std::string> &outputNames) :
    Renderer()
{
    for (std::vector<std::string>::const_iterator i = outputNames.begin(); i != outputNames.end(); ++i)
    {
        addRenderTarget(*i);
    }
}

ComputeTextureOutput::~ComputeTextureOutput()
{

}

ComputeTexture::ComputeTexture(const std::vector<std::string> &inputNames, const std::vector<std::string> &outputNames, const std::string &fragmentShaderCode) :
    input(inputNames, fragmentShaderCode),
    output(outputNames)
{
    output.addRenderable(&input);
}

ComputeTexture::~ComputeTexture()
{

}

void ComputeTexture::compute() const
{
    output.render();
}

