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
#include <algorithm>
#include <limits>

#include <tiny/mesh/staticmesh.h>

using namespace tiny;
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
    return StaticMesh::createBoxMesh(size, size, size);
}

StaticMesh StaticMesh::createBoxMesh(const float &w, const float &h, const float &d)
{
    StaticMesh mesh;
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3( w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3(-w,  h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f,-1.0f), vec3( w,  h, -d)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3(-w, -h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3( w, -h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3(-w,  h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 0.0f, 1.0f), vec3( w,  h,  d)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3(-w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3( w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3(-w, -h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f,-1.0f, 0.0f), vec3( w, -h,  d)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3(-w,  h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3( w,  h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3(-w,  h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f), vec3( 0.0f, 1.0f, 0.0f), vec3( w,  h,  d)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-w,  h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-w, -h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(-w,  h,  d)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( w, -h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( w,  h, -d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( w, -h,  d)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 1.0f, 0.0f), vec3( 1.0f, 0.0f, 0.0f), vec3( w,  h,  d)));
    
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

StaticMesh StaticMesh::createCylinderMesh(const float &radius, const float &height, const int &n)
{
    StaticMesh mesh;
    
    for (int i = 0; i <= n; ++i)
    {
        const float ifrac = static_cast<float>(i)/static_cast<float>(n);
        const float ca = cosf(2.0f*M_PI*ifrac), sa = sinf(2.0f*M_PI*ifrac);
        
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

StaticMesh StaticMesh::createIcosahedronMesh(const float &radius)
{
    StaticMesh mesh;
    
    //From http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html.
    //TODO: Fix texture coordinates.
    const float s = radius/sqrtf(0.5f*(5.0f + sqrtf(5.0f)));
    const float t = (radius*(1.0f + sqrtf(5.0f))/2.0f)/sqrtf(0.5f*(5.0f + sqrtf(5.0f)));
    
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(-s,  t, 0.0f)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3( s,  t, 0.0f)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(-s, -t, 0.0f)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3( s, -t, 0.0f)));

    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -s,  t)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  s,  t)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -s, -t)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f,  s, -t)));

    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3( t, 0.0f, -s)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3( t, 0.0f,  s)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(-t, 0.0f, -s)));
    mesh.vertices.push_back(StaticMeshVertex(vec2(1.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(-t, 0.0f,  s)));
    
    //Calculate normals and tangent vectors.
    for (std::vector<StaticMeshVertex>::iterator v = mesh.vertices.begin(); v != mesh.vertices.end(); ++v)
    {
        v->normal = normalize(v->position);
        //Any orthogonal vector to the position is tangent to the sphere and (1, 0, 0) is not part of the vertices.
        v->tangent = normalize(cross(v->position, vec3(1.0f, 0.0f, 0.0f)));
    }
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(11);
    mesh.indices.push_back(5);
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(5);
    mesh.indices.push_back(1);
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(1);
    mesh.indices.push_back(7);
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(7);
    mesh.indices.push_back(10);
    
    mesh.indices.push_back(0);
    mesh.indices.push_back(10);
    mesh.indices.push_back(11);
   
    mesh.indices.push_back(1);
    mesh.indices.push_back(5);
    mesh.indices.push_back(9);
    
    mesh.indices.push_back(5);
    mesh.indices.push_back(11);
    mesh.indices.push_back(4);
    
    mesh.indices.push_back(11);
    mesh.indices.push_back(10);
    mesh.indices.push_back(2);
    
    mesh.indices.push_back(10);
    mesh.indices.push_back(7);
    mesh.indices.push_back(6);
    
    mesh.indices.push_back(7);
    mesh.indices.push_back(1);
    mesh.indices.push_back(8);
    
    mesh.indices.push_back(3);
    mesh.indices.push_back(9);
    mesh.indices.push_back(4);
    
    mesh.indices.push_back(3);
    mesh.indices.push_back(4);
    mesh.indices.push_back(2);
    
    mesh.indices.push_back(3);
    mesh.indices.push_back(2);
    mesh.indices.push_back(6);
    
    mesh.indices.push_back(3);
    mesh.indices.push_back(6);
    mesh.indices.push_back(8);
    
    mesh.indices.push_back(3);
    mesh.indices.push_back(8);
    mesh.indices.push_back(9);

    mesh.indices.push_back(4);
    mesh.indices.push_back(9);
    mesh.indices.push_back(5);
    
    mesh.indices.push_back(2);
    mesh.indices.push_back(4);
    mesh.indices.push_back(11);
    
    mesh.indices.push_back(6);
    mesh.indices.push_back(2);
    mesh.indices.push_back(10);
    
    mesh.indices.push_back(8);
    mesh.indices.push_back(6);
    mesh.indices.push_back(7);
    
    mesh.indices.push_back(9);
    mesh.indices.push_back(8);
    mesh.indices.push_back(1);
 
    return mesh;
}

float StaticMesh::getRadius(const vec3 &scale) const
{
    //Determine the size of the mesh, scaling all vertices with a diagonal matrices with diagonal given by the provided vector.
    float radius = 0.0f;
    
    for (std::vector<StaticMeshVertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
    {
        const vec3 v = scale*i->position;
        
        radius = std::max(radius, length(v));
    }
    
    return radius;
}

std::pair<vec3, vec3> StaticMesh::getBoundingBox(const vec3 &scale) const
{
    //Determine bounding box upper and lower bound, scaling all vertices by the provided vector.
    vec3 lo = vec3(std::numeric_limits<float>::max());
    vec3 hi = vec3(std::numeric_limits<float>::min());
    
    for (std::vector<StaticMeshVertex>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
    {
        const vec3 v = scale*i->position;
        
        lo = min(lo, v);
        hi = max(hi, v);
    }
    
    return std::make_pair(lo, hi);
}

