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
#include <tiny/draw/effects/sunskyvoxelmap.h>

using namespace tiny;
using namespace tiny::draw::effects;

SunSkyVoxelMap::SunSkyVoxelMap(const int &a_nrSteps, const float &a_epsilon) :
    nrSteps(a_nrSteps),
    epsilon(a_epsilon)
{
    uniformMap.addTexture("skyTexture");
    uniformMap.addTexture("voxelTexture");
    
    skyColours.push_back(vec3(1.0f, 1.0f, 1.0f));
    setSun(normalize(vec3(1.0f, 2.0f, 1.0f)));
    setFog(3.0f);
}

SunSkyVoxelMap::~SunSkyVoxelMap()
{

}

std::string SunSkyVoxelMap::getTypeName() const {
    return "SunSkyVoxelMap";
}

std::string SunSkyVoxelMap::getFragmentShaderCode() const
{
    std::stringstream strm;
    
    strm <<
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D worldNormalTexture;\n"
"uniform sampler2D worldPositionTexture;\n"
"uniform sampler2D skyTexture;\n"
"\n"
"uniform sampler3D voxelTexture;\n"
"uniform vec3 voxelTextureSize;\n"
"uniform float voxelScale;\n"
"\n"
"uniform vec3 cameraPosition;\n"
"uniform vec3 sunDirection;\n"
"uniform vec3 fogFalloff;\n"
"uniform vec2 inverseScreenSize;\n"
"uniform vec3 skyColour;\n"
"\n"
"const float epsilon = " << epsilon << ";\n"
"\n"
"out vec4 colour;\n"
"\n"
"float castRayShadow(const vec3 direction, in vec3 position)\n"
"{\n"
"   //Prevent division by zero.\n"
"   const vec3 invDirection = 1.0f/max(abs(direction), epsilon);\n"
"   const vec3 directionSign = (step(-epsilon, direction) - 1.0f) + step(epsilon, direction);\n"
"   ivec3 voxelIndices = ivec3(floor(position/voxelScale));\n"
"   vec3 distances = max((step(0.0f, direction) - fract(position/voxelScale))*directionSign*invDirection, 0.0f);\n"
"   bvec3 mask = lessThanEqual(distances.xyz, min(distances.yzx, distances.zxy));\n"
"   \n"
"   for (int i = 0; i < " << nrSteps << "; ++i)\n"
"   {\n"
"       vec4 voxel = texelFetch(voxelTexture, voxelIndices, 0);\n"
"       \n"
"       if (voxel.x > 0.0f)\n"
"       {\n"
"           return 0.0f;\n"
"       }\n"
"       \n"
"       mask = lessThanEqual(distances.xyz, min(distances.yzx, distances.zxy));\n"
"       distances += vec3(mask)*invDirection;\n"
"       voxelIndices += ivec3(mask)*ivec3(directionSign);\n"
"   }\n"
"   \n"
"   return 1.0f;\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   vec2 tex = gl_FragCoord.xy*inverseScreenSize;\n"
"   vec3 diffuseColour = texture(diffuseTexture, tex).xyz;\n"
"   vec3 normal = texture(worldNormalTexture, tex).xyz;\n"
"   vec3 position = texture(worldPositionTexture, tex).xyz;\n"
"   float depth = texture(worldPositionTexture, tex).w;\n"
"   \n"
"   vec3 positionToCamera = normalize(cameraPosition - position);\n"
"   vec3 positionToSun = sunDirection;\n"
"   \n"
"   //Oren-Nayar shading.\n"
"   const float roughness = 1.0f;\n"
"   const float A = 1.0f - 0.5f*roughness/(roughness + 0.57f);\n"
"   const float B = 0.45f*roughness/(roughness + 0.09f);\n"
"   float ndl = dot(normal, positionToSun);\n"
"   float nde = dot(normal, positionToCamera);\n"
"   float edlperp = dot(normalize(positionToSun - dot(positionToSun, normal)*normal),\n"
"                       normalize(positionToCamera - dot(positionToCamera, normal)*normal));\n"
"   \n"
"   float diffuseFactor = max(0.0f, ndl)*(A + B*max(0.0f, edlperp)*(sqrt((1.0f - ndl*ndl)*(1.0f - nde*nde))/max(ndl, nde)));\n"
"   \n"
"   //Lambert shading.\n"
"   //diffuseFactor = max(0.0f, ndl);\n"
"   \n"
"   //Voxel shadows.\n"
"   \n"
<< (nrSteps > 0 ? 
"   diffuseFactor *= castRayShadow(positionToSun, position + epsilon*normal + 0.5f*voxelScale*vec3(voxelTextureSize.x, 0.0f, voxelTextureSize.z));\n"
 : "") <<
"   \n"
"   vec3 objectColour = 1.4f*(0.5f + diffuseFactor)*diffuseColour;\n"
"   \n"
"   //Apply distance fog.\n"
"   vec3 fogDecayFactor = vec3(exp(depth*fogFalloff));\n"
"   \n"
"   colour = vec4(skyColour*(fogDecayFactor*objectColour + (vec3(1.0f) - fogDecayFactor)), 1.0f);\n"
"   \n"
"   //Add sun and sky.\n"
"   vec3 skySunColour = texture(skyTexture, vec2(0.5f*(1.0f - positionToSun.y), 1.0f - 0.99f*max(0.01f, -positionToCamera.y))).xyz\n"
"                       + max(0.0f, positionToSun.y)*vec3(800.0f, 500.0f, 0.0f)*max(0.0f, -dot(positionToCamera, positionToSun) - 0.999f);\n"
"   \n"
"   colour.xyz = mix(colour.xyz, skySunColour, clamp(depth - 5.0e5, 0.0f, 1.0f));\n"
"   //colour.xyz = clamp(0.5f*(1.0f + position/16.0f), 0.0f, 1.0f);\n"
"}\n\0";
    
    return strm.str();
}

void SunSkyVoxelMap::setSun(const vec3 &a_sun)
{
    sun = normalize(a_sun);
    uniformMap.setVec3Uniform(sun, "sunDirection");
    uniformMap.setVec3Uniform(skyColours[static_cast<size_t>(floorf((0.5f - 0.49f*sun.y)*static_cast<float>(skyColours.size())))], "skyColour");
}

void SunSkyVoxelMap::setFog(const float &fogIntensity)
{
    uniformMap.setVec3Uniform(-fogIntensity*vec3(5.8e-6f + 2.0e-5f, 13.5e-6f + 2.0e-5f, 33.1e-6f + 2.0e-5f), "fogFalloff");
}

