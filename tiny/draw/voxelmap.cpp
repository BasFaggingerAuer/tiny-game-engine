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

VoxelMap::VoxelMap(const int &a_nrSteps, const float &a_lodScale, const float &a_epsilon) :
    ScreenFillingSquare(),
    nrSteps(a_nrSteps),
    lodScale(a_lodScale),
    epsilon(a_epsilon),
    scale(1.0f)
{
    //Setup textures.
    uniformMap.addTexture("voxelTexture");
    uniformMap.addTexture("cubemapTextureArray");
}

VoxelMap::~VoxelMap()
{

}

std::string VoxelMap::getFragmentShaderCode() const
{
    std::stringstream strm;
    
    strm <<
"#version 400\n"
"\n"
"precision highp float;\n"
"\n"
"uniform mat4 screenToWorld;\n"
"uniform mat4 worldToScreen;\n"
"uniform vec3 cameraPosition;\n"
"uniform vec2 inverseScreenSize;\n"
"\n"
"uniform sampler3D voxelTexture;\n"
"uniform vec3 voxelTextureSize;\n"
"uniform float voxelScale;\n"
"\n"
"uniform samplerCubeArray cubemapTextureArray;\n"
"\n"
"const float epsilon = " << epsilon << ";\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"float castRay(const vec3 direction, in vec3 position, out vec3 normal, out vec4 voxel, out vec3 textureDirection)\n"
"{\n"
"   //Prevent division by zero.\n"
"   const vec3 invDirection = 1.0f/max(abs(direction), epsilon);\n"
"   const vec3 directionSign = (step(-epsilon, direction) - 1.0f) + step(epsilon, direction);\n"
"   ivec3 voxelIndices = ivec3(floor(position/voxelScale));\n"
"   float baseDist = 0.0f;\n"
"   \n"
"   for (int i = 0; i < 4; ++i)\n"
"   {\n"
"       voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"       baseDist += max(0.0f, 255.0f*voxel.z - epsilon);\n"
"       voxelIndices = ivec3(floor((position/voxelScale) + baseDist*direction));\n"
"   }\n"
"   \n"
"   vec3 distances = max((step(0.0f, direction) - fract((position/voxelScale) + baseDist*direction))*directionSign*invDirection, 0.0f);\n"
"   bvec3 mask = lessThanEqual(distances.xyz, min(distances.yzx, distances.zxy));\n"
"   float dist = baseDist;\n"
"   \n"
"   normal = -directionSign*vec3(mask);\n"
"   \n"
"   for (int i = 0; i < " << nrSteps << "; ++i)\n"
"   {\n"
"       voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"       \n"
"       if (voxel.y == 0.0f) return -1.0f;"
"       \n"
"       if (voxel.x > 0.0f)\n"
"       {\n"
"           textureDirection = (position/voxelScale) + dist*direction - vec3(voxelIndices) - vec3(0.5f);\n"
"           return dist*voxelScale;\n"
"       }\n"
"       \n"
"       mask = lessThanEqual(distances.xyz, min(distances.yzx, distances.zxy));\n"
"       normal = -directionSign*vec3(mask);\n"
"       dist = baseDist + dot(vec3(mask), distances);\n"
"       distances += vec3(mask)*invDirection;\n"
"       voxelIndices += ivec3(mask)*ivec3(directionSign);\n"
"   }\n"
"   \n"
"   return -1.0f;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   vec4 tmp = screenToWorld*vec4(2.0f*inverseScreenSize*gl_FragCoord.xy - 1.0f, -1.0f, 1.0f);\n"
"   vec3 direction = normalize(((tmp/tmp.w) - vec4(cameraPosition, 0.0f)).xyz);\n"
"   vec3 normal;\n"
"   vec4 voxel;\n"
"   vec3 textureDirection;\n"
"   float dist = castRay(direction, cameraPosition + 0.5f*voxelScale*vec3(voxelTextureSize.x, 0.0f, voxelTextureSize.z), normal, voxel, textureDirection);\n"
"   \n"
"   if (dist < 0.0f) discard;\n"
"   \n"
"   worldPosition = vec4(cameraPosition + dist*direction, 1.0f);\n"
"   tmp = worldToScreen*worldPosition;\n"
"   \n"
"   //diffuse = vec4(voxel.xyz, 1.0f);\n"
"   //diffuse = texture(cubemapTextureArray, vec4(normalize(textureDirection), 255.0f*voxel.x - 1.0f));\n"
"   diffuse = 2.0f*voxel.y*textureLod(cubemapTextureArray, vec4(normalize(textureDirection), 255.0f*voxel.x - 1.0f), max(0.0f, log2(tmp.z)/" << lodScale << "));\n"
"   worldNormal = vec4(normalize(normal), 2.0f);\n"
"   worldPosition.w = tmp.z;\n"
"   \n"
"   gl_FragDepth = (log(C*tmp.z + E) / log(C*D + E));\n"
"}\n\0";
    
    return strm.str();
}

float VoxelMap::getScale() const
{
    return scale;
}

