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

SquareVertexBufferInterpreter::SquareVertexBufferInterpreter() :
    VertexBufferInterpreter<ScreenVertex>(4)
{
    hostData[0] = ScreenVertex(vec2(-1.0f, 1.0f), vec2(0.0f, 1.0f));
    hostData[1] = ScreenVertex(vec2(-1.0f,-1.0f), vec2(0.0f, 0.0f));
    hostData[2] = ScreenVertex(vec2( 1.0f, 1.0f), vec2(1.0f, 1.0f));
    hostData[3] = ScreenVertex(vec2( 1.0f,-1.0f), vec2(1.0f, 0.0f));
    sendToDevice();
    
    addVec2Attribute(0*sizeof(float), "vertex");
    addVec2Attribute(2*sizeof(float), "textureCoordinate");
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
        uniformMap.addTexture(*i);
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
    //Draw screen-filling quad.
    square.bind(program);
    renderRangeAsTriangleStrip(0, 4);
    square.unbind(program);
}

ComputeTextureOutput::ComputeTextureOutput(const std::vector<std::string> &outputNames, const int &textureWidth, const int &textureHeight) :
    Renderer(false, textureWidth, textureHeight)
{
    for (std::vector<std::string>::const_iterator i = outputNames.begin(); i != outputNames.end(); ++i)
    {
        addRenderTarget(*i);
    }
}

ComputeTextureOutput::~ComputeTextureOutput()
{

}

ComputeTexture::ComputeTexture(const std::vector<std::string> &inputNames, const int &outputTextureWidth, const int &outputTextureHeight, const std::vector<std::string> &outputNames, const std::string &fragmentShaderCode) :
    input(inputNames, fragmentShaderCode),
    output(outputNames, outputTextureWidth, outputTextureHeight)
{
    output.addRenderable(0, &input, false, false);
}

ComputeTexture::~ComputeTexture()
{

}

void ComputeTexture::compute() const
{
    output.render();
}

UniformMap &ComputeTexture::uniformMap()
{
    return input.uniformMap;
}

