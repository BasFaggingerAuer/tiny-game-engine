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
#include <iostream>
#include <exception>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>

#include <tiny/math/vec.h>
#include <tiny/mesh/io/staticmesh.h>

using namespace tiny;
using namespace tiny::mesh;

StaticMesh tiny::mesh::io::readStaticMesh(const std::string &fileName)
{
	//Use AssImp to read all data from the file.
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Quality);
	
	if (!scene)
	{
		std::cerr << "Unable to read '" << fileName << "' from disk!" << std::endl;
		throw std::exception();
	}
}

struct compareivec3
{
	inline bool operator () (const ivec3 &a, const ivec3 &b) const
	{
		if (a.x == b.x)
		{
			if (a.y == b.y) return (a.z < b.z);
			else return (a.y < b.y);
		}
		
		return a.x < b.x;
	};
};

StaticMesh tiny::mesh::io::readStaticMeshOBJ(const std::string &fileName, const bool &centreMesh)
{
    std::ifstream file(fileName.c_str());
    StaticMesh mesh;
    
    if (!file.good())
    {
        std::cerr << "Unable to read '" << fileName << "' from disk!" << std::endl;
        throw std::exception();
    }
    
    //Read Wavefront OBJ mesh file data.
    bool recalculatedNormals = false;
    std::vector<vec2> texCoords;
    std::vector<vec3> normals;
    std::vector<vec3> vertices;
    std::vector<ivec3> indices;
    
    while (file.good())
    {
        std::string line;
        
        getline(file, line);
        
        if (line.length() < 2) continue;
        
        std::istringstream lineStream(line);
        std::string tag;
        
        lineStream >> tag;
        
        if (tag == "#")
        {

        }
        else if (tag == "vt")
        {
            vec2 v;
            
            lineStream >> v.x >> v.y;
            
            v.y = 1.0f - v.y;
            
            texCoords.push_back(v);
        }
        else if (tag == "vn")
        {
            vec3 v;
            
            lineStream >> v.x >> v.y >> v.z;
            normals.push_back(normalize(v));
        }
        else if (tag == "v")
        {
            vec3 v;
            
            lineStream >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (tag == "f")
        {
            int newNormal = -1;
            
            for (int i = 0; i < 3; ++i)
            {
                std::string idxLine;
                
                lineStream >> idxLine;
                
                size_t pos;
                
                while ((pos = idxLine.find("/")) != std::string::npos) idxLine[pos] = ' ';
                
                std::istringstream idxStream(idxLine);
                ivec3 v;
                
                idxStream >> v.x >> v.y >> v.z;
                
                v -= ivec3(1, 1, 1);
                
                //Has a normal vector been specified?
                if (v.z < 0)
                {
                    if (newNormal < 0)
                    {
                        newNormal = normals.size();
                        normals.push_back(vec3(0.0f, 0.0f, 1.0f));
                    }
                    
                    v.z = newNormal;
                }
                
                if (v.x < 0 || v.y < 0 || v.z < 0 || v.x >= (int)vertices.size() || v.y >= (int)texCoords.size() || v.z >= (int)normals.size())
                {
                    std::cerr << "Invalid triangle indices " << v << " (" << line << ", " << idxLine << ")!" << std::endl;
                    throw std::exception();
                }
                
                indices.push_back(v);
            }
            
            //Do we need to recalculate the normal vector?
            if (newNormal >= 0)
            {
                const vec3 a = vertices[indices[indices.size() - 3].x];
                const vec3 b = vertices[indices[indices.size() - 2].x];
                const vec3 c = vertices[indices[indices.size() - 1].x];
                
                recalculatedNormals = true;
                normals[newNormal] = normalize(cross(b - a, c - a));
            }
        }
    }
    
    if (recalculatedNormals)
    {
        std::cerr << "Normal vectors have been recalculated!" << std::endl;
    }
    
    if (vertices.empty() || texCoords.empty() || normals.empty() || indices.empty())
    {
        std::cerr << "Empty model!" << std::endl;
        throw std::exception();
    }
    
    //Centre the mesh.
    if (centreMesh)
    {
        vec3 minVec = vertices[0], maxVec = vertices[0];
        
        for (std::vector<vec3>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
        {
            minVec = min(minVec, *i);
            maxVec = max(maxVec, *i);
        }
        
        const vec3 centreVec = 0.5f*(minVec + maxVec);
        
        for (std::vector<vec3>::iterator i = vertices.begin(); i != vertices.end(); ++i)
        {
            *i -= centreVec;
        }
    }
    
    //Compound vertices.
    std::map<ivec3, unsigned int, compareivec3> convertMap;
    
    mesh.indices.reserve(indices.size());
    mesh.vertices.reserve(vertices.size());
    
    for (std::vector<ivec3>::const_iterator i = indices.begin(); i != indices.end(); ++i)
    {
        std::map<ivec3, unsigned int, compareivec3>::iterator j = convertMap.find(*i);
        
        if (j == convertMap.end())
        {
            //We have a new triplet of vertex, texture vertex, and normal.
            mesh.indices.push_back(convertMap[*i] = mesh.vertices.size());
            mesh.vertices.push_back(StaticMeshVertex(texCoords[i->y], normals[i->z], vertices[i->x]));
        }
        else
        {
            //We already encountered this triplet.
            mesh.indices.push_back(j->second);
        }
    }
    
    std::cerr << "Read a mesh with " << vertices.size() << " vertices, " << texCoords.size() << " texture vertices, " << normals.size() << " normals, and " << indices.size() << " indices as " << mesh.vertices.size() << " compounded vertices and " << mesh.indices.size() << " indices from '" << fileName << "'." << std::endl;
    
    return mesh;
}

