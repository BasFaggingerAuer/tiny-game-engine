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

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

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

const aiMesh *getAiMesh(const aiScene *scene, const std::string &meshName)
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
        throw std::exception();
    }
    
    return sourceMesh;
}

template<typename MeshType, typename MeshVertexType>
void copyAiMeshVertices(const aiMesh *sourceMesh, MeshType &mesh)
{
    //Copy vertices.
    mesh.vertices.resize(sourceMesh->mNumVertices);
    
    for (unsigned int i = 0; i < mesh.vertices.size(); ++i)
    {
        mesh.vertices[i] = MeshVertexType(vec2(sourceMesh->mTextureCoords[0][i].x, sourceMesh->mTextureCoords[0][i].y),
                                          vec3(sourceMesh->mTangents[i].x, sourceMesh->mTangents[i].y, sourceMesh->mTangents[i].z),
                                          vec3(sourceMesh->mNormals[i].x, sourceMesh->mNormals[i].y, sourceMesh->mNormals[i].z),
                                          vec3(sourceMesh->mVertices[i].x, sourceMesh->mVertices[i].y, sourceMesh->mVertices[i].z));
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

} //namespace detail

} //namespace io

} //namespace mesh

} //namespace tiny
