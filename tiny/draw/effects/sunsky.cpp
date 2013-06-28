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
#include <tiny/draw/effects/sunsky.h>

using namespace tiny::draw::effects;

SunSky::SunSky()
{
    uniformMap.addTexture("skyTexture");
    
    skyColours.push_back(vec3(1.0f, 1.0f, 1.0f));
    setSun(normalize(vec3(1.0f, 2.0f, 1.0f)));
    setFog(3.0f);
}

SunSky::~SunSky()
{

}

std::string SunSky::getFragmentShaderCode() const
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
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   vec2 tex = gl_FragCoord.xy*inverseScreenSize;\n"
"   vec4 diffuse = texture(diffuseTexture, tex);\n"
"   vec4 worldNormal = texture(worldNormalTexture, tex);\n"
"   vec4 worldPosition = texture(worldPositionTexture, tex);\n"
"   \n"
"   vec3 relativePosition = normalize(worldPosition.xyz - cameraPosition);\n"
"   vec3 sunContribution = max(0.0f, sunDirection.y)*vec3(800.0f, 500.0f, 0.0f)*max(0.0f, dot(relativePosition, sunDirection) - 0.999f);\n"
"   \n"
"   float depth = worldPosition.w;\n"
"   float directLight = 0.25f + 1.5f*max(dot(worldNormal.xyz, sunDirection), 0.0f);\n"
"   vec3 decay = vec3(exp(depth*fogFalloff));\n"
"   \n"
"	vec4 skyDirectColour = texture(skyTexture, vec2(0.5f*(1.0f - sunDirection.y), 1.0f - max(0.01f, normalize(worldPosition).y)));\n"
"   \n"
"   colour = vec4((diffuse.xyz*directLight*decay + (vec3(1.0f) - decay))*skyColour, 1.0f);\n"
"   colour = mix(colour, vec4(skyDirectColour.xyz + sunContribution, 1.0f), clamp(depth - 5.0e5, 0.0f, 1.0f));\n"
"}\n");
}

void SunSky::setSun(const vec3 &a_sun)
{
    sun = normalize(a_sun);
    uniformMap.setVec3Uniform(sun, "sunDirection");
    uniformMap.setVec3Uniform(skyColours[static_cast<size_t>(floor((0.5f - 0.49f*sun.y)*static_cast<float>(skyColours.size())))], "skyColour");
}

void SunSky::setFog(const float &fogIntensity)
{
    uniformMap.setVec3Uniform(-fogIntensity*vec3(5.8e-6 + 2.0e-5, 13.5e-6 + 2.0e-5, 33.1e-6 + 2.0e-5), "fogFalloff");
}

