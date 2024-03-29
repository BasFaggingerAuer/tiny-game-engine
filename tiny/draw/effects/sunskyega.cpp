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
#include <tiny/draw/effects/sunskyega.h>

using namespace tiny;
using namespace tiny::draw::effects;

SunSkyEGA::SunSkyEGA()
{
    uniformMap.addTexture("skyTexture");
    
    skyColours.push_back(vec3(1.0f, 1.0f, 1.0f));
    setSun(normalize(vec3(1.0f, 2.0f, 1.0f)));
    setFog(3.0f);
}

SunSkyEGA::~SunSkyEGA()
{

}

std::string SunSkyEGA::getTypeName() const {
    return "SunSkyEGA";
}

std::string SunSkyEGA::getFragmentShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D worldNormalTexture;\n"
"uniform sampler2D worldPositionTexture;\n"
"uniform sampler2D skyTexture;\n"
"\n"
"uniform vec3 cameraPosition;\n"
"uniform vec3 sunDirection;\n"
"uniform vec3 fogFalloff;\n"
"uniform vec2 inverseScreenSize;\n"
"uniform vec3 skyColour;\n"
"\n"
"const float[64] bayer_dither = float[](0, 48, 12, 60, 3, 51, 15, 63,\n"
"    32, 16, 44, 28, 35, 19, 47, 31,\n"
"    8,  56, 4,  52, 11, 59, 7,  55,\n"
"    40, 24, 36, 20, 43, 27, 39, 23,\n"
"    2,  50, 14, 62, 1,  49, 13, 61,\n"
"    34, 18, 46, 30, 33, 17, 45, 29,\n"
"    10, 58, 6,  54, 9,  57, 5,  53,\n"
"    42, 26, 38, 22, 41, 25, 37, 21);\n"
"\n"
"out vec4 colour;\n"
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
"   ivec2 ditherPos = ivec2(mod(gl_FragCoord.xy, 8.0f));\n"
"   colour.xyz += (bayer_dither[ditherPos.x + 8*ditherPos.y]/64.0f - 0.5f)/4.0f;\n"
"   colour.xyz = round(3.0f*clamp(colour.xyz, 0.0f, 1.0f))/3.0f;\n"
"}\n");
}

void SunSkyEGA::setSun(const vec3 &a_sun)
{
    sun = normalize(a_sun);
    uniformMap.setVec3Uniform(sun, "sunDirection");
    uniformMap.setVec3Uniform(skyColours[static_cast<size_t>(floorf((0.5f - 0.49f*sun.y)*static_cast<float>(skyColours.size())))], "skyColour");
}

void SunSkyEGA::setFog(const float &fogIntensity)
{
    uniformMap.setVec3Uniform(-fogIntensity*vec3(5.8e-6f + 2.0e-5f, 13.5e-6f + 2.0e-5f, 33.1e-6f + 2.0e-5f), "fogFalloff");
}

