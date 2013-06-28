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
#include <tiny/draw/staticmeshhorde.h>

using namespace tiny::draw;

StaticMeshInstanceVertexBufferInterpreter::StaticMeshInstanceVertexBufferInterpreter(const size_t &nrMeshes) :
    VertexBufferInterpreter<StaticMeshInstance>(nrMeshes)
{
    addVec4Attribute(0*sizeof(float), "v_positionAndSize");
    addVec4Attribute(4*sizeof(float), "v_orientation");
}

StaticMeshInstanceVertexBufferInterpreter::~StaticMeshInstanceVertexBufferInterpreter()
{

}

StaticMeshHorde::StaticMeshHorde(const tiny::mesh::StaticMesh &mesh, const size_t &a_maxNrMeshes) :
    Renderable(),
    nrVertices(mesh.vertices.size()),
    nrIndices(mesh.indices.size()),
    maxNrMeshes(a_maxNrMeshes),
    nrMeshes(0),
    indices(mesh),
    vertices(mesh),
    meshes(maxNrMeshes)
{
    uniformMap.addTexture("diffuseTexture");
}

StaticMeshHorde::~StaticMeshHorde()
{

}

std::string StaticMeshHorde::getVertexShaderCode() const
{
    return
"#version 150\n"
"\n"
"uniform mat4 worldToScreen;\n"
"\n"
"in vec2 v_textureCoordinate;\n"
"in vec3 v_normal;\n"
"in vec3 v_position;\n"
"\n"
"in vec4 v_positionAndSize;\n"
"in vec4 v_orientation;\n"
"\n"
"out vec2 f_tex;\n"
"out vec3 f_worldNormal;\n"
"out vec3 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"vec3 qtransform(const vec4 q, const vec3 v)\n"
"{\n"
"   return (v + 2.0f*cross(cross(v, q.xyz) + q.w*v, q.xyz));\n"
"}\n"
"\n"
"void main(void)\n"
"{\n"
"   f_tex = v_textureCoordinate;\n"
"   f_worldNormal = v_normal;\n"
"   f_worldPosition = v_position;\n"
"   gl_Position = worldToScreen*vec4(v_positionAndSize.w*qtransform(v_orientation, v_position) + v_positionAndSize.xyz, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0";
}

std::string StaticMeshHorde::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"\n"
"const float C = 1.0f, D = 1.0e8, E = 1.0f;\n"
"\n"
"in vec2 f_tex;\n"
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
"   worldNormal = vec4(normalize(f_worldNormal), 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0";
}

void StaticMeshHorde::render(const ShaderProgram &program) const
{
    vertices.bind(program);
    meshes.bind(program, 1);
    renderIndicesAsTrianglesInstanced(indices, nrMeshes);
    vertices.unbind(program);
}

