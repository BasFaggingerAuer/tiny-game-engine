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
#include <tiny/draw/animatedmeshhorde.h>

using namespace tiny::draw;

AnimatedMeshInstanceVertexBufferInterpreter::AnimatedMeshInstanceVertexBufferInterpreter(const size_t &nrMeshes) :
    VertexBufferInterpreter<AnimatedMeshInstance>(nrMeshes)
{
    addVec4Attribute(0*sizeof(float), "v_positionAndSize");
    addVec4Attribute(4*sizeof(float), "v_orientation");
    addIVec2Attribute(8*sizeof(float), "v_animationFrames");
}

AnimatedMeshInstanceVertexBufferInterpreter::~AnimatedMeshInstanceVertexBufferInterpreter()
{

}

AnimatedMeshHorde::AnimatedMeshHorde(const tiny::mesh::AnimatedMesh &mesh, const size_t &a_maxNrMeshes) :
    Renderable(),
    nrVertices(mesh.vertices.size()),
    nrIndices(mesh.indices.size()),
    maxNrMeshes(a_maxNrMeshes),
    nrMeshes(0),
    indices(mesh),
    vertices(mesh),
    meshes(maxNrMeshes)
{
    uniformMap.addTexture("animationTexture");
    uniformMap.addTexture("diffuseTexture");
    uniformMap.addTexture("normalTexture");
}

AnimatedMeshHorde::~AnimatedMeshHorde()
{

}

std::string AnimatedMeshHorde::getTypeName() const {
    return "AnimatedMeshHorde";
}

std::string AnimatedMeshHorde::getVertexShaderCode() const
{
    return
"#version 150\n"
"\n"
"uniform mat4 worldToScreen;\n"
"uniform samplerBuffer animationTexture;\n"
"\n"
"in vec2 v_textureCoordinate;\n"
"in vec3 v_tangent;\n"
"in vec3 v_normal;\n"
"in vec3 v_position;\n"
"in vec4 v_weights;\n"
"in ivec4 v_bones;\n"
"\n"
"in vec4 v_positionAndSize;\n"
"in vec4 v_orientation;\n"
"in ivec2 v_animationFrames;\n"
"\n"
"out vec2 f_tex;\n"
"out vec3 f_worldTangent;\n"
"out vec3 f_worldNormal;\n"
"out vec3 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"vec3 qtransform(const vec4 q, const vec3 v)\n"
"{\n"
"   return v + 2.0f*cross(q.xyz, cross(q.xyz, v) + q.w*v);\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   f_tex = v_textureCoordinate;\n"
"   f_worldTangent = vec3(0.0f);\n"
"   f_worldNormal = vec3(0.0f);\n"
"   f_worldPosition = vec3(0.0f);\n"
"   \n"
"   for (int i = 0; i < 4; ++i)\n"
"   {\n"
"       vec3 scale = texelFetch(animationTexture, v_animationFrames.x + 3*v_bones[i] + 0).xyz;\n"
"       vec4 rotate = texelFetch(animationTexture, v_animationFrames.x + 3*v_bones[i] + 1);\n"
"       vec3 translate = texelFetch(animationTexture, v_animationFrames.x + 3*v_bones[i] + 2).xyz;\n"
"       \n"
"       f_worldTangent += v_weights[i]*qtransform(rotate, v_tangent);\n"
"       f_worldNormal += v_weights[i]*qtransform(rotate, v_normal);\n"
"       f_worldPosition += v_weights[i]*(qtransform(rotate, scale*v_position) + translate);\n"
"   }\n"
"   \n"
"   f_worldTangent = qtransform(v_orientation, f_worldTangent);\n"
"   f_worldNormal = qtransform(v_orientation, f_worldNormal);\n"
"   f_worldPosition = v_positionAndSize.w*qtransform(v_orientation, f_worldPosition) + v_positionAndSize.xyz;\n"
"   gl_Position = worldToScreen*vec4(f_worldPosition, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0";
}

std::string AnimatedMeshHorde::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"uniform sampler2D normalTexture;\n"
"\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"in vec2 f_tex;\n"
"in vec3 f_worldTangent;\n"
"in vec3 f_worldNormal;\n"
"in vec3 f_worldPosition;\n"
"in float f_cameraDepth;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void main(void)\n"
"{\n"
"   diffuse = texture(diffuseTexture, f_tex);\n"
"   vec3 normal = 2.0f*texture(normalTexture, f_tex).xyz - vec3(1.0f);\n"
"   \n"
"   if (diffuse.w < 0.5f) discard;\n"
"   \n"
"   worldNormal = vec4(normalize(mat3(f_worldTangent, cross(f_worldNormal, f_worldTangent), f_worldNormal)*normal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
}

void AnimatedMeshHorde::render(const ShaderProgram &program) const
{
    vertices.bind(program);
    meshes.bind(program, 1);
    renderIndicesAsTrianglesInstanced(indices, nrMeshes);
    meshes.unbind(program);
    vertices.unbind(program);
}

