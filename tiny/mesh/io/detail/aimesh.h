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
#pragma once

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include <tiny/math/vec.h>

using namespace tiny;
using namespace tiny::mesh;

namespace tiny
{

namespace mesh
{

namespace io
{

namespace detail
{

inline mat4 aiMatrixToMat4(const aiMatrix4x4 &m)
{
    return mat4(m.a1, m.a2, m.a3, m.a4,
                m.b1, m.b2, m.b3, m.b4,
                m.c1, m.c2, m.c3, m.c4,
                m.d1, m.d2, m.d3, m.d4);
}

inline const aiMesh *getAiMesh(const aiScene *scene, const std::string &meshName)
{
    //Obtain the mesh with the largest number of vertices in the file and the given name.
    if (!scene->HasMeshes())
    {
        std::cerr << "The scene does not contain any meshes!" << std::endl;
        throw std::exception();
    }
    
    unsigned int largestNrVertices = scene->mMeshes[0]->mNumVertices;
    unsigned int largestIndex = 0;
    
    for (unsigned int i = 1; i < scene->mNumMeshes; ++i)
    {
        if (scene->mMeshes[i]->mNumVertices > largestNrVertices &&
            (meshName == "" || meshName == std::string(scene->mMeshes[i]->mName.data)))
        {
            largestNrVertices = scene->mMeshes[i]->mNumVertices;
            largestIndex = i;
        }
    }
    
    const aiMesh *sourceMesh = scene->mMeshes[largestIndex];
    
    if (sourceMesh->mNumVertices <= 0 ||
        !sourceMesh->HasPositions() ||
        !sourceMesh->HasFaces() ||
        !sourceMesh->HasTangentsAndBitangents() ||
        !sourceMesh->HasNormals() ||
        !sourceMesh->HasTextureCoords(0))
    {
        std::cerr << "Mesh '" << meshName << "' in does not possess vertex/face/normal/texture data!" << std::endl;
        
        std::cerr << "Number of vertices : " << sourceMesh->mNumVertices << std::endl;
        std::cerr << "Positions          : " << sourceMesh->HasPositions() << std::endl;
        std::cerr << "Faces              : " << sourceMesh->HasFaces() << std::endl;
        std::cerr << "Tangents           : " << sourceMesh->HasTangentsAndBitangents() << std::endl;
        std::cerr << "Normals            : " << sourceMesh->HasNormals() << std::endl;
        std::cerr << "Texture coordinates: " << sourceMesh->HasTextureCoords(0) << std::endl;
    
        throw std::exception();
    }
    
    return sourceMesh;
}

template<typename MeshType, typename MeshVertexType>
void copyAiMeshVertices(const aiMesh *sourceMesh, MeshType &mesh, const aiMatrix4x4 &transformation)
{
    //Find rotation for normals and tangents.
    aiVector3D scaling;
    aiQuaternion rotation;
    aiVector3D translation;
    
    transformation.Decompose(scaling, rotation, translation);
    
    const aiMatrix3x3 rotationMatrix = rotation.GetMatrix();
    
    //Copy vertices.
    mesh.vertices.resize(sourceMesh->mNumVertices);
    
    for (unsigned int i = 0; i < mesh.vertices.size(); ++i)
    {
        const aiVector3D t = rotationMatrix*sourceMesh->mTangents[i];
        const aiVector3D n = rotationMatrix*sourceMesh->mNormals[i];
        const aiVector3D v = transformation*sourceMesh->mVertices[i];
        
        mesh.vertices[i] = MeshVertexType(vec2(sourceMesh->mTextureCoords[0][i].x, sourceMesh->mTextureCoords[0][i].y),
                                          vec3(t.x, t.y, t.z),
                                          vec3(n.x, n.y, n.z),
                                          vec3(v.x, v.y, v.z));
    }
}

template<typename MeshType>
void copyAiMeshIndices(const aiMesh *sourceMesh, MeshType &mesh)
{
    //Copy indices.
    mesh.indices.resize(3*sourceMesh->mNumFaces);
    
    for (unsigned int i = 0; i < sourceMesh->mNumFaces; ++i)
    {
        const aiFace *face = &sourceMesh->mFaces[i];
        
        if (face->mNumIndices != 3)
        {
            std::cerr << "Mesh does not consist of triangles!" << std::endl;
            throw std::exception();
        }
        
        mesh.indices[3*i + 0] = face->mIndices[0];
        mesh.indices[3*i + 1] = face->mIndices[1];
        mesh.indices[3*i + 2] = face->mIndices[2];
    }
}

inline void setAiNodePointers(const aiNode *node, std::map<std::string, const aiNode *> &nodeNameToPointer)
{
    //Does the node name already exist?
    if (!nodeNameToPointer.insert(std::make_pair(node->mName.data, node)).second)
    {
        std::cerr << "Node name '" << node->mName.data << "' is not unique!" << std::endl;
        throw std::exception();
    }
    
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        setAiNodePointers(node->mChildren[i], nodeNameToPointer);
    }
}

inline void updateAiNodeMatrices(aiNode *node, const aiMatrix4x4 &transformation, std::map<std::string, aiMatrix4x4> &nodeNameToMatrix)
{
    assert(node);
    
    std::map<std::string, aiMatrix4x4>::iterator nodeIterator = nodeNameToMatrix.find(node->mName.data);
    
    assert(nodeIterator != nodeNameToMatrix.end());
    
    const aiMatrix4x4 newTransformation = transformation*nodeIterator->second;
    
    nodeIterator->second = newTransformation;
    
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        updateAiNodeMatrices(node->mChildren[i], newTransformation, nodeNameToMatrix);
    }
}

} //namespace detail

} //namespace io

} //namespace mesh

} //namespace tiny
