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
#include <tiny/draw/iconhorde.h>

using namespace tiny;
using namespace tiny::draw;

ScreenIconVertexBufferInterpreter::ScreenIconVertexBufferInterpreter(const size_t &nrIcons) :
    VertexBufferInterpreter<ScreenIconInstance>(nrIcons)
{
    addVec4Attribute(0*sizeof(float), "v_positionAndSize");
    addVec4Attribute(4*sizeof(float), "v_icon");
    addVec4Attribute(8*sizeof(float), "v_colour");
}

ScreenIconVertexBufferInterpreter::~ScreenIconVertexBufferInterpreter()
{

}

ScreenIconHorde::ScreenIconHorde(const size_t &a_maxNrIcons) :
    maxNrIcons(a_maxNrIcons),
    nrIcons(0),
    icons(a_maxNrIcons)
{
    uniformMap.addTexture("iconTexture");
}

ScreenIconHorde::~ScreenIconHorde()
{

}

std::string ScreenIconHorde::getVertexShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"in vec4 v_positionAndSize;\n"
"in vec4 v_icon;\n"
"in vec4 v_colour;\n"
"\n"
"out vec4 g_icon;\n"
"out vec4 g_colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    gl_Position = v_positionAndSize;\n"
"    g_icon = v_icon;\n"
"    g_colour = v_colour;\n"
"}\n\0");
}

std::string ScreenIconHorde::getGeometryShaderCode() const
{
    return std::string(
"#version 150\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
"\n"
"layout (points) in;\n"
"layout (triangle_strip) out;\n"
"layout (max_vertices = 4) out;\n"
"\n"
"in vec4 g_icon[];\n"
"in vec4 g_colour[];\n"
"\n"
"out vec2 tex;\n"
"out vec4 f_colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec2 w = vec2(gl_PositionIn[0].z, 0.0f);\n"
"    vec2 h = vec2(0.0f, gl_PositionIn[0].w);\n"
"    \n"
"   f_colour = g_colour[0];\n"
"    \n"
"    tex = vec2(g_icon[0].x, g_icon[0].y + g_icon[0].w);\n"
"    gl_Position = vec4(gl_PositionIn[0].xy, 0.0f, 1.0f);\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y + g_icon[0].w);\n"
"    gl_Position = vec4(gl_PositionIn[0].xy + w, 0.0f, 1.0f);\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(g_icon[0].x, g_icon[0].y);\n"
"    gl_Position = vec4(gl_PositionIn[0].xy + h, 0.0f, 1.0f);\n"
"    EmitVertex();\n"
"    \n"
"    tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y);\n"
"    gl_Position = vec4(gl_PositionIn[0].xy + w + h, 0.0f, 1.0f);\n"
"    EmitVertex();\n"
"    \n"
"    EndPrimitive();\n"
"}\n\0");
}

std::string ScreenIconHorde::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D iconTexture;\n"
"\n"
"in vec2 tex;\n"
"in vec4 f_colour;\n"
"\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    colour = texture(iconTexture, tex)*f_colour;\n"
"    gl_FragDepth = 0.0f;\n"
"}\n\0";
}

void ScreenIconHorde::render(const ShaderProgram &program) const
{
    icons.bind(program);
    renderRangeAsPoints(0, nrIcons);
    icons.unbind(program);
}

void ScreenIconHorde::setText(const float &x, const float &y, const float &size, const float &aspectRatio, const std::string &text, const IconTexture2D &map)
{
    //Draw font.
    const float sizeScale = size/map.getMaxIconDimensions().y;
    vec4 pos = vec4(x, y, 0.0f, 0.0f);
    vec4 colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    
    nrIcons = 0;
    
    for (std::string::const_iterator i = text.begin(); i != text.end() && nrIcons < maxNrIcons; ++i)
    {
        pos.z = 0.5f*size;
        pos.w = size;
        
        if (*i == '\n')
        {
            pos.x = x;
            pos.y += size;
        }
        else if (*i == '\r') pos.x = x;
        else if (*i == '\b') pos.x -= pos.z;
        else if (*i == '\\')
        {
            //We have found an escaped character.
            ++i;
            
            if (i != text.end())
            {
                if (*i == 'r') colour = vec4(1.0f, 0.0f, 0.0f, 1.0f);
                else if (*i == 'g') colour = vec4(0.0f, 1.0f, 0.0f, 1.0f);
                else if (*i == 'b') colour = vec4(0.0f, 0.0f, 1.0f, 1.0f);
                else if (*i == 'w') colour = vec4(1.0f, 1.0f, 1.0f, 1.0f);
                else if (*i == 'y') colour = vec4(1.0f, 1.0f, 0.0f, 1.0f);
                else if (*i == 'c') colour = vec4(0.0f, 1.0f, 1.0f, 1.0f);
                else if (*i == 'p') colour = vec4(1.0f, 0.0f, 1.0f, 1.0f);
                else if (*i == '0') intensity = 0.0f;
                else if (*i == '1') intensity = 0.25f;
                else if (*i == '2') intensity = 0.5f;
                else if (*i == '3') intensity = 0.75f;
                else if (*i == '4') intensity = 1.0f;
                else
                {
                    const vec4 icon = map.getIcon(*i);
                    
                    pos.w = sizeScale*icon.w;
                    pos.z = pos.w*(icon.z/icon.w)/aspectRatio;
                    icons[nrIcons++] = ScreenIconInstance(pos, icon, intensity*colour);
                    pos.x += pos.z;
                }
            }
            else
            {
                break;
            }
        }
        else if (*i == '\0') break;
        else
        {
            const vec4 icon = map.getIcon(*i);
            
            pos.w = sizeScale*icon.w;
            pos.z = pos.w*(icon.z/icon.w)/aspectRatio;
            icons[nrIcons++] = ScreenIconInstance(pos, icon, intensity*colour);
            pos.x += pos.z;
        }
    }
    
    //Update buffer.
    icons.sendToDevice();
}

