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
#include <tiny/mesh/staticmesh.h>

using namespace tiny::mesh;

StaticMesh::StaticMesh() :
    vertices(),
    indices()
{

}

StaticMesh::~StaticMesh()
{

}

StaticMesh StaticMesh::createCubeMesh(const float &size)
{
    StaticMesh mesh;
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3( size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-size,  size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3( size,  size, -size)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3(-size, -size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3( size, -size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3(-size,  size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3( size,  size,  size)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3(-size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3( size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3(-size, -size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3( size, -size,  size)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3(-size,  size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3( size,  size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3(-size,  size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3( size,  size,  size)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-size,  size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-size, -size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-size,  size,  size)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( size, -size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( size,  size, -size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( size, -size,  size)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( size,  size,  size)));
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(3);
    mesh.indices.push_back(1);
    mesh.indices.push_back(0);
    mesh.indices.push_back(2);
    mesh.indices.push_back(3);
    
    mesh.indices.push_back(4);
    mesh.indices.push_back(5);
    mesh.indices.push_back(7);
    mesh.indices.push_back(4);
    mesh.indices.push_back(7);
    mesh.indices.push_back(6);
    
    mesh.indices.push_back(8);
    mesh.indices.push_back(9);
    mesh.indices.push_back(11);
    mesh.indices.push_back(8);
    mesh.indices.push_back(11);
    mesh.indices.push_back(10);
    
    mesh.indices.push_back(12);
    mesh.indices.push_back(15);
    mesh.indices.push_back(13);
    mesh.indices.push_back(12);
    mesh.indices.push_back(14);
    mesh.indices.push_back(15);
    
    mesh.indices.push_back(16);
    mesh.indices.push_back(19);
    mesh.indices.push_back(17);
    mesh.indices.push_back(16);
    mesh.indices.push_back(18);
    mesh.indices.push_back(19);
    
    mesh.indices.push_back(20);
    mesh.indices.push_back(21);
    mesh.indices.push_back(23);
    mesh.indices.push_back(20);
    mesh.indices.push_back(23);
    mesh.indices.push_back(22);
    
    return mesh;
}

StaticMesh StaticMesh::createCylinderMesh(const float &radius, const float &height)
{
    const int n = 12;
    
    StaticMesh mesh;
    
    for (int i = 0; i <= n; ++i)
    {
        const float ifrac = static_cast<float>(i)/static_cast<float>(n);
        const float ca = cos(2.0f*M_PI*ifrac), sa = sin(2.0f*M_PI*ifrac);
        
        mesh.vertices.push_back(StaticMeshVertex(vec2(ifrac, 0.0f), vec3(sa, 0.0f, -ca), vec3(ca, 0.0f, sa), vec3(radius*ca, 0.0f, radius*sa)));
        mesh.vertices.push_back(StaticMeshVertex(vec2(ifrac, 1.0f), vec3(sa, 0.0f, -ca), vec3(ca, 0.0f, sa), vec3(radius*ca, height, radius*sa)));
    }
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3(0.0f,  1.0f, 0.0f), vec3(0.0f, height, 0.0f)));
    
    for (int i = 0; i < n; ++i)
    {
        mesh.indices.push_back(2*i + 1);
        mesh.indices.push_back(2*i + 2);
        mesh.indices.push_back(2*i);
        
        mesh.indices.push_back(2*i + 1);
        mesh.indices.push_back(2*i + 3);
        mesh.indices.push_back(2*i + 2);
    }
        
    for (int i = 0; i < n; ++i)
    {
        mesh.indices.push_back(2*n + 2);
        mesh.indices.push_back(2*i);
        mesh.indices.push_back(2*i + 2);
    }
    
    for (int i = 0; i < n; ++i)
    {
        mesh.indices.push_back(2*i + 3);
        mesh.indices.push_back(2*i + 1);
        mesh.indices.push_back(2*n + 3);
    }
    
    return mesh;
}

