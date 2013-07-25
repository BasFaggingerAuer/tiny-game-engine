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
#include <tiny/mesh/io/animatedmesh.h>

using namespace tiny;
using namespace tiny::mesh;

AnimatedMesh tiny::mesh::io::readAnimatedMesh(const std::string &fileName, const std::string &meshName)
{
    //Use AssImp to read all data from the file.
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Quality);
    
    if (!scene)
    {
        std::cerr << "Unable to read '" << fileName << "' from disk!" << std::endl;
        throw std::exception();
    }
    
    //Obtain the mesh with the largest number of vertices in the file and the given name.
    if (!scene->mMeshes || scene->mNumMeshes < 1)
    {
        std::cerr << "'" << fileName << "' does not contain any meshes!" << std::endl;
        throw std::exception();
    }
    
    unsigned int largestNrVertices = scene->mMeshes[0]->mNumVertices;
    unsigned int largestIndex = 0;
    
    for (unsigned int i = 1; i < scene->mNumMeshes; ++i)
    {
        if (scene->mMeshes[i]->mNumVertices > largestNrVertices)
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
        !sourceMesh->HasTextureCoords(0) ||
        !sourceMesh->HasBones())
    {
        std::cerr << "Mesh '" << meshName << "' in '" << fileName << "' does not possess vertex/face/normal/texture/bone data!" << std::endl;
        throw std::exception();
    }
    
    AnimatedMesh mesh;
    
    //Copy vertices.
    mesh.vertices.resize(sourceMesh->mNumVertices);
    
    for (unsigned int i = 0; i < mesh.vertices.size(); ++i)
    {
        mesh.vertices[i] = AnimatedMeshVertex(vec2(sourceMesh->mTextureCoords[0][i].x, sourceMesh->mTextureCoords[0][i].y),
                                            vec3(sourceMesh->mTangents[i].x, sourceMesh->mTangents[i].y, sourceMesh->mTangents[i].z),
                                            vec3(sourceMesh->mNormals[i].x, sourceMesh->mNormals[i].y, sourceMesh->mNormals[i].z),
                                            vec3(sourceMesh->mVertices[i].x, sourceMesh->mVertices[i].y, sourceMesh->mVertices[i].z));
    }
    
    //Copy bones.
    std::vector<unsigned int> vertexBoneCounts(mesh.vertices.size(), 0);
    
    mesh.skeleton.bones.resize(sourceMesh->mNumBones);
    
    for (unsigned int i = 0; i < mesh.skeleton.bones.size(); ++i)
    {
        const aiBone *bone = sourceMesh->mBones[i];
        const aiMatrix4x4 m = bone->mOffsetMatrix;
        
        mesh.skeleton.bones[i] = Bone(std::string(bone->mName.data),
                                      mat4(m.a1, m.a2, m.a3, m.a4,
                                           m.b1, m.b2, m.b3, m.b4,
                                           m.c1, m.c2, m.c3, m.c4,
                                           m.d1, m.d2, m.d3, m.d4));
        
        //Retrieve weights for vertices affected by this bone.
        for (unsigned int j = 0; j < bone->mNumWeights; ++j)
        {
            const unsigned int id = bone->mWeights[j].mVertexId;
            const float weight = bone->mWeights[j].mWeight;
            
            if (id >= mesh.vertices.size() || weight < 0.0f || weight > 1.0f)
            {
                std::cerr << "Mesh '" << meshName << "' in '" << fileName << "' contains invalid bone indices " << id << " or weights " << weight << "!" << std::endl;
                throw std::exception();
            }
            
            //Discard small weights.
            if (weight <= 0.001f)
            {
                continue;
            }
            
            const unsigned int count = vertexBoneCounts[id];
            
            if (count >= 4)
            {
                std::cerr << "Warning: mesh '" << meshName << "' in '" << fileName << "' contains more than four bones per vertex, discarding excess data!" << std::endl;
            }
            else
            {
                AnimatedMeshVertex *v = &mesh.vertices[id];
                
                if (count == 0)
                {
                    v->weights.x = weight;
                    v->bones.x = i;
                }
                else if (count == 1)
                {
                    v->weights.y = weight;
                    v->bones.y = i;
                }
                else if (count == 2)
                {
                    v->weights.z = weight;
                    v->bones.z = i;
                }
                else if (count == 3)
                {
                    v->weights.w = weight;
                    v->bones.w = i;
                }
                    
                vertexBoneCounts[id]++;
            }
        }
    }
    
    //Copy indices.
    mesh.indices.resize(3*sourceMesh->mNumFaces);
    
    for (unsigned int i = 0; i < sourceMesh->mNumFaces; ++i)
    {
        const aiFace *face = &sourceMesh->mFaces[i];
        
        if (face->mNumIndices != 3)
        {
            std::cerr << "Mesh '" << meshName << "' in '" << fileName << "' does not consist of triangles!" << std::endl;
            throw std::exception();
        }
        
        mesh.indices[3*i + 0] = face->mIndices[0];
        mesh.indices[3*i + 1] = face->mIndices[1];
        mesh.indices[3*i + 2] = face->mIndices[2];
    }
    
    std::cerr << "Read mesh '" << meshName << "' with " << mesh.vertices.size() << " vertices, " << mesh.indices.size()/3 << " triangles, " << mesh.skeleton.bones.size() << " bones, and " << mesh.skeleton.animations.size() << " animations from '" << fileName << "'." << std::endl;
    
    //importer will go out of scope, which will free all read data automatically.
    return mesh;
}

