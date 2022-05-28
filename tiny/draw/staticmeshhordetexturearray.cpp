/*
Copyright 2021, Bas Fagginger Auer.

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
#include <tiny/draw/staticmeshhordetexturearray.h>

using namespace tiny::draw;

StaticMeshHordeTextureArray::StaticMeshHordeTextureArray(const tiny::mesh::StaticMesh &mesh, const size_t &a_maxNrMeshes) :
    Renderable(),
    nrVertices(mesh.vertices.size()),
    nrIndices(mesh.indices.size()),
    maxNrMeshes(a_maxNrMeshes),
    nrMeshes(0),
    indices(mesh),
    vertices(mesh),
    meshes(maxNrMeshes)
{
    uniformMap.addTexture("diffuseTextureArray");
    uniformMap.addTexture("normalTextureArray");
}

StaticMeshHordeTextureArray::~StaticMeshHordeTextureArray()
{

}

std::string StaticMeshHordeTextureArray::getTypeName() const {
    return "StaticMeshHordeTextureArray";
}

std::string StaticMeshHordeTextureArray::getVertexShaderCode() const
{
    return
"#version 150\n"
"\n"
"uniform mat4 worldToScreen;\n"
"\n"
"in vec2 v_textureCoordinate;\n"
"in vec3 v_tangent;\n"
"in vec3 v_normal;\n"
"in vec3 v_position;\n"
"\n"
"in vec4 v_positionAndSize;\n"
"in vec4 v_orientation;\n"
"in vec4 v_colorMultiplier;\n"
"\n"
"out vec2 f_tex;\n"
"out vec3 f_worldTangent;\n"
"out vec3 f_worldNormal;\n"
"out vec3 f_worldPosition;\n"
"out vec4 f_colorMultiplier;\n"
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
"   f_worldTangent = qtransform(v_orientation, v_tangent);\n"
"   f_worldNormal = qtransform(v_orientation, v_normal);\n"
"   f_worldPosition = v_positionAndSize.w*qtransform(v_orientation, v_position) + v_positionAndSize.xyz;\n"
"   f_colorMultiplier = v_colorMultiplier;\n"
"   gl_Position = worldToScreen*vec4(f_worldPosition, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0";
}

std::string StaticMeshHordeTextureArray::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2DArray diffuseTextureArray;\n"
"uniform sampler2DArray normalTextureArray;\n"
"\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"in vec2 f_tex;\n"
"in vec3 f_worldTangent;\n"
"in vec3 f_worldNormal;\n"
"in vec3 f_worldPosition;\n"
"in vec4 f_colorMultiplier;\n"
"in float f_cameraDepth;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void main(void)\n"
"{\n"
"   diffuse = texture(diffuseTextureArray, vec3(f_tex, f_colorMultiplier.w))*vec4(f_colorMultiplier.xyz, 1.0f);\n"
"   vec3 normal = 2.0f*texture(normalTextureArray, vec3(f_tex, f_colorMultiplier.w)).xyz - vec3(1.0f);\n"
"   \n"
"   if (diffuse.w < 0.5f) discard;\n"
"   \n"
"   worldNormal = vec4(normalize(mat3(f_worldTangent, cross(f_worldNormal, f_worldTangent), f_worldNormal)*normal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
}

void StaticMeshHordeTextureArray::render(const ShaderProgram &program) const
{
    vertices.bind(program);
    meshes.bind(program, 1);
    renderIndicesAsTrianglesInstanced(indices, nrMeshes);
    meshes.unbind(program);
    vertices.unbind(program);
}

