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
#include <tiny/draw/staticmesh.h>

using namespace tiny::draw;

StaticMeshVertexBufferInterpreter::StaticMeshVertexBufferInterpreter(const tiny::mesh::StaticMesh &mesh) :
    VertexBufferInterpreter<tiny::mesh::StaticMeshVertex>(mesh.vertices.begin(), mesh.vertices.end())
{
    addVec2Attribute(0*sizeof(float), "v_textureCoordinate");
    addVec3Attribute(2*sizeof(float), "v_normal");
    addVec3Attribute(5*sizeof(float), "v_position");
}

StaticMeshVertexBufferInterpreter::~StaticMeshVertexBufferInterpreter()
{

}

StaticMeshIndexBuffer::StaticMeshIndexBuffer(const tiny::mesh::StaticMesh &mesh) :
    IndexBuffer<unsigned int>(mesh.indices.begin(), mesh.indices.end())
{

}

StaticMeshIndexBuffer::~StaticMeshIndexBuffer()
{

}

StaticMesh::StaticMesh(const tiny::mesh::StaticMesh &mesh) :
    Renderable(),
    nrVertices(mesh.vertices.size()),
    nrIndices(mesh.indices.size()),
    indices(mesh),
    vertices(mesh)
{
    uniformMap.addTexture("diffuseTexture");
}

StaticMesh::~StaticMesh()
{

}

std::string StaticMesh::getVertexShaderCode() const
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
"out vec2 f_tex;\n"
"out vec3 f_worldNormal;\n"
"out vec3 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"void main(void)\n"
"{\n"
"   f_tex = v_textureCoordinate;\n"
"   f_worldNormal = v_normal;\n"
"   f_worldPosition = v_position;\n"
"   gl_Position = worldToScreen*vec4(v_position, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0";
}

std::string StaticMesh::getFragmentShaderCode() const
{
    return
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"\n"
"const float C = 1.0f, D = 1.0e6, E = 1.0f;\n"
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

void StaticMesh::render(const ShaderProgram &program) const
{
    vertices.bind(program);
    //renderRangeAsPoints(0, nrVertices);
    renderIndicesAsTriangles(indices);
    vertices.unbind(program);
}

