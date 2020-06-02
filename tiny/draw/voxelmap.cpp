/*
Copyright 2020, Bas Fagginger Auer.

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
#include <sstream>
#include <tiny/draw/voxelmap.h>

using namespace tiny;
using namespace tiny::draw;
using namespace tiny::draw::detail;

VoxelMap::VoxelMap(const int &a_nrVoxelLODs, const int &a_nrSteps) :
    ScreenFillingSquare(),
    nrVoxelLODs(a_nrVoxelLODs),
    nrSteps(a_nrSteps)
{
    //Setup textures.
    uniformMap.addTexture("voxelTexture");
}

VoxelMap::~VoxelMap()
{

}

std::string VoxelMap::getFragmentShaderCode() const
{
    std::stringstream strm;
    
    strm <<
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D tangentTexture;\n"
"uniform sampler2D farTangentTexture;\n"
"uniform sampler2D normalTexture;\n"
"uniform sampler2D farNormalTexture;\n"
"\n"
"uniform sampler2D attributeTexture;\n"
"uniform sampler2D farAttributeTexture;\n"
"uniform sampler2DArray localDiffuseTexture;\n"
"uniform sampler2DArray localNormalTexture;\n"
"uniform vec2 diffuseScale;\n"
"\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"in vec3 f_worldPosition;\n"
"in vec4 f_texturePosition;\n"
"in float f_farMorphFactor;\n"
"in float f_cameraDepth;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void main(void)\n"
"{\n"
"   vec4 att = mix(texture(attributeTexture, f_texturePosition.xy),\n"
"                  texture(farAttributeTexture, f_texturePosition.zw),\n"
"                  f_farMorphFactor);\n"
"   vec3 f_worldTangent = normalize(2.0f*mix(\n"
"                            texture(tangentTexture, f_texturePosition.xy),\n"
"                            texture(farTangentTexture, f_texturePosition.zw),\n"
"                            f_farMorphFactor).xyz - vec3(1.0f));\n"
"   vec3 f_worldNormal = normalize(2.0f*mix(\n"
"                            texture(normalTexture, f_texturePosition.xy),\n"
"                            texture(farNormalTexture, f_texturePosition.zw),\n"
"                            f_farMorphFactor).xyz - vec3(1.0f));\n"
"   \n"
"   att.x *= 255.0f;\n"
"   vec3 normal = mix(texture(localNormalTexture, vec3(diffuseScale*f_texturePosition.xy, floor(att.x))),\n"
"                     texture(localNormalTexture, vec3(diffuseScale*f_texturePosition.xy, floor(att.x) + 1.0f)),\n"
"                     att.x - floor(att.x)).xyz;\n"
"   normal = 2.0f*normal - vec3(1.0f);\n"
"   diffuse = mix(texture(localDiffuseTexture, vec3(diffuseScale*f_texturePosition.xy, floor(att.x))),\n"
"                 texture(localDiffuseTexture, vec3(diffuseScale*f_texturePosition.xy, floor(att.x) + 1.0f)),\n"
"                 att.x - floor(att.x));\n"
"   worldNormal = vec4(normalize(mat3(f_worldTangent, cross(f_worldNormal, f_worldTangent), f_worldNormal)*normal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
    
    return strm.str();
}

