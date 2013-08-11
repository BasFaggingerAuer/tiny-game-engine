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
        std::cerr << "Unable to read '" << fileName << "' from disk:" << importer.GetErrorString() << "!" << std::endl;
        throw std::exception();
    }
    
    //Create map from node names to their pointers.
    std::map<std::string, const aiNode *> nodeNameToPointer;
    
    detail::setAiNodePointers(scene->mRootNode, nodeNameToPointer);
    
    //We need to keep track of all transformation matrices.
    std::map<std::string, aiMatrix4x4> nodeNameToMatrix;
    
    for (std::map<std::string, const aiNode *>::const_iterator i = nodeNameToPointer.begin(); i != nodeNameToPointer.end(); ++i)
    {
        nodeNameToMatrix.insert(std::make_pair(i->first, nodeNameToPointer[i->first]->mTransformation));
    }
    
    //Find transformations recursively.
    detail::updateAiNodeMatrices(scene->mRootNode, aiMatrix4x4(), nodeNameToMatrix);
    
    //Retrieve mesh.
    const aiMesh *sourceMesh = detail::getAiMesh(scene, meshName);
    StaticMesh mesh;
    
    assert(sourceMesh);
    
    //Find transformation corresponding to this mesh.
    aiMatrix4x4 transformation;
    
    for (std::map<std::string, const aiNode *>::const_iterator i = nodeNameToPointer.begin(); i != nodeNameToPointer.end(); ++i)
    {
        for (unsigned int j = 0; j < i->second->mNumMeshes; ++j)
        {
            //Does this node apply to this mesh?
            if (scene->mMeshes[i->second->mMeshes[j]] == sourceMesh)
            {
                transformation = nodeNameToMatrix[i->first];
                break;
            }
        }
    }
    
    //Read mesh and apply the found transformation.
    detail::copyAiMeshVertices<StaticMesh, StaticMeshVertex>(sourceMesh, mesh, transformation);
    detail::copyAiMeshIndices(sourceMesh, mesh);
    
    std::cerr << "Read mesh '" << meshName << "' with " << mesh.vertices.size() << " vertices and " << mesh.indices.size()/3 << " triangles from '" << fileName << "'." << std::endl;
    
    //importer will go out of scope, which will free all read data automatically.
    return mesh;
}

