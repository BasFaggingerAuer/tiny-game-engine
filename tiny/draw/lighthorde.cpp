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
#include <tiny/draw/lighthorde.h>

using namespace tiny;
using namespace tiny::draw;

PointLightVertexBufferInterpreter::PointLightVertexBufferInterpreter(const size_t &nrLights) :
    VertexBufferInterpreter(),
    instances(nrLights)
{
    addVec4Attribute(instances, 0*sizeof(float), "v_positionAndSize");
    addVec4Attribute(instances, 4*sizeof(float), "v_colour");
}

PointLightVertexBufferInterpreter::~PointLightVertexBufferInterpreter()
{

}

PointLightHorde::PointLightHorde(const size_t &a_maxNrLights) :
    maxNrLights(a_maxNrLights),
    nrLights(0),
    lights(a_maxNrLights)
{
    
}

PointLightHorde::~PointLightHorde()
{

}

std::string PointLightHorde::getVertexShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"in vec4 v_positionAndSize;\n"
"in vec4 v_colour;\n"
"\n"
"out vec4 g_colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_Position = v_positionAndSize;\n"
"    g_colour = v_colour;\n"
"}\n\0");
}

std::string PointLightHorde::getGeometryShaderCode() const
{
    return std::string(
"#version 150\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
"\n"
"layout (points) in;\n"
"layout (triangle_strip) out;\n"
"layout (max_vertices = 4) out;\n"
"\n"
"uniform mat4 worldToScreen;\n"
"uniform vec3 cameraPosition;\n"
"\n"
"in vec4 g_colour[];\n"
"\n"
"out vec2 tex;\n"
"out vec4 f_position;\n"
"out vec4 f_colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec2 delta = normalize(gl_PositionIn[0].xz - cameraPosition.xz);\n"
"    float lightRadius = gl_PositionIn[0].w;\n"
"    vec4 w = lightRadius*vec4(-delta.y, 0.0f, delta.x, 0.0f);\n"
"    vec4 h = lightRadius*vec4(0.0f, 1.0f, 0.0f, 0.0f);\n"
"    \n"
"    gl_PositionIn[0].w = 1.0f;\n"
"    f_colour = g_colour[0];\n"
"    \n"
"    tex = vec2(0.0f, 1.0f);\n"
"    gl_Position = gl_PositionIn[0] - w;\n"
"    gl_Position = f_position = worldToScreen*gl_Position;\n"
"   f_position.w = lightRadius;\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(1.0f, 1.0f);\n"
"    gl_Position = gl_PositionIn[0] + w;\n"
"    gl_Position = f_position = worldToScreen*gl_Position;\n"
"   f_position.w = lightRadius;\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(0.0f, 0.0f);\n"
"    gl_Position = gl_PositionIn[0] - w + h;\n"
"    gl_Position = f_position = worldToScreen*gl_Position;\n"
"   f_position.w = lightRadius;\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(1.0f, 0.0f);\n"
"    gl_Position = gl_PositionIn[0] + w + h;\n"
"    gl_Position = f_position = worldToScreen*gl_Position;\n"
"   f_position.w = lightRadius;\n"
"    EmitVertex();\n"
"    \n"
"    EndPrimitive();\n"
"}\n\0");
}

std::string PointLightHorde::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D worldNormalTexture;\n"
"uniform sampler2D worldPositionTexture;\n"
"\n"
"in vec2 tex;\n"
"in vec4 f_position;\n"
"in vec4 f_colour;\n"
"\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   vec4 diffuse = texture(diffuseTexture, tex);\n"
"   vec4 worldNormal = texture(worldNormalTexture, tex);\n"
"   vec4 worldPosition = texture(worldPositionTexture, tex);\n"
"   \n"
"   float fallOff = max(0.0f, 1.0f - length(worldPosition.xyz - f_position.xyz)/f_position.w);\n"
"   \n"
"   colour = vec4(1.0f, fallOff, 1.0f, 1.0f)*f_colour;\n"
"}\n\0";
}

void PointLightHorde::render(const ShaderProgram &program) const
{
    lights.bind(program);
    renderRangeAsPoints(0, nrLights);
    lights.unbind(program);
}

