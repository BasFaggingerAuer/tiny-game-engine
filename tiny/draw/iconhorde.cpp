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
    Renderable(),
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

void ScreenIconHorde::appendText(vec4 & pos, const float &size, const float &aspectRatio, const std::string &text, const IconTexture2D &map, const Colour &colour, const vec4 &boxSize)
{
    //Draw font.
    const float sizeScale = size/map.getMaxIconDimensions().y;
    
    for (std::string::const_iterator i = text.begin(); i != text.end() && nrIcons < maxNrIcons; ++i)
    {
        const vec4 icon = map.getIcon(*i);
        
        pos.w = sizeScale*icon.w;
        pos.z = pos.w*(icon.z/icon.w)/aspectRatio;
        icons[nrIcons++] = ScreenIconInstance(pos, icon, colour.toVector());
        pos.x += pos.z;
        if(pos.x > boxSize.z) // Try wrapping when adding out-of-box characters
        {
            if(pos.y < boxSize.w + size) break; // Cannot wrap beyond bottom of box
            const vec4 iconSpace = map.getIcon(' ');
            for(unsigned int j = nrIcons-1; j < maxNrIcons; j--)
            {
                if(icons[j].icon == iconSpace)
                {
                    float shift = icons[j+1].positionAndSize.x - boxSize.x;
                    float newStart = icons[nrIcons-1].positionAndSize.x - icons[j+1].positionAndSize.x+pos.z;
                    if(j+1 == nrIcons) newStart = 0.0f; // if space exceeds box, we can start new line at margin
                    for(unsigned int k = j+1; k < nrIcons; k++)
                    {
                        icons[k].positionAndSize.x -= shift;
                        icons[k].positionAndSize.y -= size;
                    }
                    pos.x = boxSize.x+newStart;
                    pos.y -= size;
                    break;
                }
            }
        }
    }
    
    //Update buffer.
    icons.sendToDevice();
}

void ScreenIconHorde::eraseText(void)
{
    nrIcons = 0;
    for(size_t i = 0; i < maxNrIcons; i++) icons[i] = ScreenIconInstance();
    icons.sendToDevice();
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

WorldIconVertexBufferInterpreter::WorldIconVertexBufferInterpreter(const size_t &nrIcons) :
    VertexBufferInterpreter<WorldIconInstance>(nrIcons)
{
    addVec4Attribute(0*sizeof(float), "v_position");
    addVec2Attribute(4*sizeof(float), "v_size");
    addVec4Attribute(6*sizeof(float), "v_icon");
    addVec4Attribute(10*sizeof(float), "v_colour");
}

WorldIconVertexBufferInterpreter::~WorldIconVertexBufferInterpreter()
{

}

WorldIconHorde::WorldIconHorde(const size_t &a_maxNrIcons, const bool &a_fullIconRotation) :
    Renderable(),
    fullIconRotation(a_fullIconRotation),
    maxNrIcons(a_maxNrIcons),
    nrIcons(0),
    icons(a_maxNrIcons)
{
    uniformMap.addTexture("iconTexture");
}

WorldIconHorde::~WorldIconHorde()
{

}

std::string WorldIconHorde::getVertexShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"in vec4 v_position;\n"
"in vec2 v_size;\n"
"in vec4 v_icon;\n"
"in vec4 v_colour;\n"
"\n"
"out vec2 g_size;\n"
"out vec4 g_icon;\n"
"out vec4 g_colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   gl_Position = vec4(v_position.xyz, 1.0f);\n"
"   g_size = v_size;\n"
"   g_icon = v_icon;\n"
"   g_colour = v_colour;\n"
"}\n\0");
}

std::string WorldIconHorde::getGeometryShaderCode() const
{
    //Depending on whether we want all icon sprites to face the camera (rotate over all axes) or to just rotate over the y-axis we supply different geometry shaders.
    if (fullIconRotation)
    {
    return std::string(
"#version 150\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
"\n"
"layout (points) in;\n"
"layout (triangle_strip) out;\n"
"layout (max_vertices = 4) out;\n"
"\n"
"uniform vec3 cameraPosition;\n"
"uniform mat4 cameraToWorld;\n"
"uniform mat4 worldToScreen;\n"
"\n"
"in vec2 g_size[];\n"
"in vec4 g_icon[];\n"
"in vec4 g_colour[];\n"
"\n"
"out vec2 f_tex;\n"
"out vec4 f_colour;\n"
"out vec3 f_worldNormal;\n"
"out vec4 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec4 w = cameraToWorld*vec4(g_size[0].x, 0.0f, 0.0f, 0.0f);\n"
"    vec4 h = cameraToWorld*vec4(0.0f, g_size[0].y, 0.0f, 0.0f);\n"
"    \n"
"    f_colour = g_colour[0];\n"
"    f_worldNormal = (cameraToWorld*vec4(0.0f, 0.0f, -1.0f, 0.0f)).xyz;\n"
"    \n"
"    f_tex = vec2(g_icon[0].x, g_icon[0].y + g_icon[0].w);\n"
"    f_worldPosition = gl_PositionIn[0] - w - h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y + g_icon[0].w);\n"
"    f_worldPosition = gl_PositionIn[0] + w - h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x, g_icon[0].y);\n"
"    f_worldPosition = gl_PositionIn[0] - w + h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y);\n"
"    f_worldPosition = gl_PositionIn[0] + w + h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    EndPrimitive();\n"
"}\n\0");
    }
    else
    {
    return std::string(
"#version 150\n"
"#extension GL_EXT_geometry_shader4 : enable\n"
"\n"
"layout (points) in;\n"
"layout (triangle_strip) out;\n"
"layout (max_vertices = 4) out;\n"
"\n"
"uniform vec3 cameraPosition;\n"
"uniform mat4 cameraToWorld;\n"
"uniform mat4 worldToScreen;\n"
"\n"
"in vec2 g_size[];\n"
"in vec4 g_icon[];\n"
"in vec4 g_colour[];\n"
"\n"
"out vec2 f_tex;\n"
"out vec4 f_colour;\n"
"out vec3 f_worldNormal;\n"
"out vec4 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec2 delta = normalize(gl_PositionIn[0].xz - cameraPosition.xz);\n"
"    vec4 w = g_size[0].x*vec4(-delta.y, 0.0f, delta.x, 0.0f);\n"
"    vec4 h = g_size[0].y*vec4(0.0f, 1.0f, 0.0f, 0.0f);\n"
"    \n"
"    f_colour = g_colour[0];\n"
"    f_worldNormal = vec3(-delta.x, 0.0f, -delta.y);\n"
"    \n"
"    f_tex = vec2(g_icon[0].x, g_icon[0].y + g_icon[0].w);\n"
"    f_worldPosition = gl_PositionIn[0] - w - h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y + g_icon[0].w);\n"
"    f_worldPosition = gl_PositionIn[0] + w - h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x, g_icon[0].y);\n"
"    f_worldPosition = gl_PositionIn[0] - w + h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    f_tex = vec2(g_icon[0].x + g_icon[0].z, g_icon[0].y);\n"
"    f_worldPosition = gl_PositionIn[0] + w + h;\n"
"    gl_Position = worldToScreen*f_worldPosition;\n"
"    f_cameraDepth = gl_Position.z;\n"
"    EmitVertex();\n"
"    \n"
"    EndPrimitive();\n"
"}\n\0");
    }
}

std::string WorldIconHorde::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D iconTexture;\n"
"\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"in vec2 f_tex;\n"
"in vec4 f_colour;\n"
"in vec3 f_worldNormal;\n"
"in vec4 f_worldPosition;\n"
"in float f_cameraDepth;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void main(void)\n"
"{\n"
"   diffuse = texture(iconTexture, f_tex)*f_colour;\n"
"   \n"
"   if (diffuse.w < 0.5f) discard;\n"
"   \n"
"   worldNormal = vec4(normalize(f_worldNormal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition.xyz, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
}

void WorldIconHorde::render(const ShaderProgram &program) const
{
    icons.bind(program);
    renderRangeAsPoints(0, nrIcons);
    icons.unbind(program);
}

void WorldIconHorde::setText(const float &x, const float &y, const float &size, const std::string &text, const IconTexture2D &map)
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
                    pos.z = pos.w*(icon.z/icon.w);
                    icons[nrIcons++] = WorldIconInstance(vec3(pos.x, 0.0f, pos.y), vec2(pos.z, pos.w), icon, intensity*colour);
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
            pos.z = pos.w*(icon.z/icon.w);
            icons[nrIcons++] = WorldIconInstance(vec3(pos.x, 0.0f, pos.y), vec2(pos.z, pos.w), icon, intensity*colour);
            pos.x += pos.z;
        }
    }
    
    //Update buffer.
    icons.sendToDevice();
}

