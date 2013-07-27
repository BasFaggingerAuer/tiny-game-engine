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
#include <tiny/draw/animatedmesh.h>

using namespace tiny::draw;

AnimatedMeshVertexBufferInterpreter::AnimatedMeshVertexBufferInterpreter(const tiny::mesh::AnimatedMesh &mesh) :
    VertexBufferInterpreter<tiny::mesh::AnimatedMeshVertex>(mesh.vertices.begin(), mesh.vertices.end())
{
    addVec2Attribute(0*sizeof(float), "v_textureCoordinate");
    addVec3Attribute(2*sizeof(float), "v_tangent");
    addVec3Attribute(5*sizeof(float), "v_normal");
    addVec3Attribute(8*sizeof(float), "v_position");
    addVec4Attribute(11*sizeof(float), "v_weights");
    addIVec4Attribute(15*sizeof(float), "v_bones");
}

AnimatedMeshVertexBufferInterpreter::~AnimatedMeshVertexBufferInterpreter()
{

}

AnimatedMeshIndexBuffer::AnimatedMeshIndexBuffer(const tiny::mesh::AnimatedMesh &mesh) :
    IndexBuffer<unsigned int>(mesh.indices.begin(), mesh.indices.end())
{

}

AnimatedMeshIndexBuffer::~AnimatedMeshIndexBuffer()
{

}

AnimationTextureBuffer::AnimationTextureBuffer() :
    keyFrameBuffer(1, GL_TEXTURE_BUFFER, GL_STATIC_DRAW)
{

}

AnimationTextureBuffer::~AnimationTextureBuffer()
{

}

AnimatedMesh::AnimatedMesh(const tiny::mesh::AnimatedMesh &mesh) :
    Renderable(),
    indices(mesh),
    vertices(mesh),
    nrBones(mesh.skeleton.bones.size())
{
    uniformMap.addTexture("animationTexture");
    uniformMap.addTexture("diffuseTexture");
    uniformMap.addTexture("normalTexture");
    setAnimationFrame(0);
}

AnimatedMesh::~AnimatedMesh()
{

}

void AnimatedMesh::setAnimationFrame(const int &a_frame)
{
    uniformMap.setIntUniform(3*nrBones*a_frame, "animationFrame");
}

std::string AnimatedMesh::getVertexShaderCode() const
{
    return
"#version 150\n"
"\n"
"uniform mat4 worldToScreen;\n"
"uniform samplerBuffer animationTexture;\n"
"uniform int animationFrame;\n"
"\n"
"in vec2 v_textureCoordinate;\n"
"in vec3 v_tangent;\n"
"in vec3 v_normal;\n"
"in vec3 v_position;\n"
"in vec4 v_weights;\n"
"in ivec4 v_bones;\n"
"\n"
"out vec2 f_tex;\n"
"out vec3 f_worldTangent;\n"
"out vec3 f_worldNormal;\n"
"out vec3 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"vec3 qtransform(const vec4 q, const vec3 v)\n"
"{\n"
"	return (v + 2.0f*cross(cross(v, q.xyz) + q.w*v, q.xyz));\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   vec4 scaleAndTime = vec4(0.0f);\n"
"   vec4 rotate = vec4(0.0f);\n"
"   vec4 translate = vec4(0.0f);\n"
"   \n"
"   scaleAndTime += v_weights.x*texelFetch(animationTexture, animationFrame + 3*v_bones.x + 0);\n"
"   rotate += v_weights.x*texelFetch(animationTexture, animationFrame + 3*v_bones.x + 1);\n"
"   translate += v_weights.x*texelFetch(animationTexture, animationFrame + 3*v_bones.x + 2);\n"
"   \n"
"   scaleAndTime += v_weights.y*texelFetch(animationTexture, animationFrame + 3*v_bones.y + 0);\n"
"   rotate += v_weights.y*texelFetch(animationTexture, animationFrame + 3*v_bones.y + 1);\n"
"   translate += v_weights.y*texelFetch(animationTexture, animationFrame + 3*v_bones.y + 2);\n"
"   \n"
"   scaleAndTime += v_weights.z*texelFetch(animationTexture, animationFrame + 3*v_bones.z + 0);\n"
"   rotate += v_weights.z*texelFetch(animationTexture, animationFrame + 3*v_bones.z + 1);\n"
"   translate += v_weights.z*texelFetch(animationTexture, animationFrame + 3*v_bones.z + 2);\n"
"   \n"
"   scaleAndTime += v_weights.w*texelFetch(animationTexture, animationFrame + 3*v_bones.w + 0);\n"
"   rotate += v_weights.w*texelFetch(animationTexture, animationFrame + 3*v_bones.w + 1);\n"
"   translate += v_weights.w*texelFetch(animationTexture, animationFrame + 3*v_bones.w + 2);\n"
"   \n"
"   normalize(rotate);\n"
"   \n"
"   f_tex = v_textureCoordinate;\n"
"   f_worldTangent = qtransform(rotate, v_tangent);\n"
"   f_worldNormal = qtransform(rotate, v_normal);\n"
"   f_worldPosition = qtransform(rotate, v_position) + translate.xyz;\n"
"   gl_Position = worldToScreen*vec4(f_worldPosition, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0";
}

std::string AnimatedMesh::getFragmentShaderCode() const
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
"   \n"
"   if (diffuse.w < 0.5f) discard;\n"
"   \n"
"   vec3 normal = 2.0f*texture(normalTexture, f_tex).xyz - vec3(1.0f);\n"
"   \n"
"   worldNormal = vec4(normalize(mat3(f_worldTangent, cross(f_worldNormal, f_worldTangent), f_worldNormal)*normal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
}

void AnimatedMesh::render(const ShaderProgram &program) const
{
    vertices.bind(program);
    renderIndicesAsTriangles(indices);
    vertices.unbind(program);
}

