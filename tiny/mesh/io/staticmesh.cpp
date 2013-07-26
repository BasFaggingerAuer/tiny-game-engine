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
#include <string>
#include <vector>
#include <cassert>

#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>

#include <tiny/math/vec.h>
#include <tiny/mesh/io/staticmesh.h>
#include <tiny/mesh/io/detail/aimesh.h>

using namespace tiny;
using namespace tiny::mesh;

StaticMesh tiny::mesh::io::readStaticMesh(const std::string &fileName, const std::string &meshName)
{
    //Use AssImp to read all data from the file.
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Quality);
    
    if (!scene)
    {
        std::cerr << "Unable to read '" << fileName << "' from disk!" << std::endl;
        throw std::exception();
    }
    
    const aiMesh *sourceMesh = detail::getAiMesh(scene, meshName);
    StaticMesh mesh;
    
    assert(sourceMesh);
    detail::copyAiMeshVertices<StaticMesh, StaticMeshVertex>(sourceMesh, mesh);
    detail::copyAiMeshIndices(sourceMesh, mesh);
    
    std::cerr << "Read mesh '" << meshName << "' with " << mesh.vertices.size() << " vertices and " << mesh.indices.size()/3 << " triangles from '" << fileName << "'." << std::endl;
    
    //importer will go out of scope, which will free all read data automatically.
    return mesh;
}

