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
#include <map>
#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/mesh/io/animatedmesh.h>
#include <tiny/mesh/io/detail/aimesh.h>

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

void copyAiMeshBones(const aiMesh *sourceMesh, AnimatedMesh &mesh)
{
    //Copy bones.
    std::vector<unsigned int> vertexBoneCounts(mesh.vertices.size(), 0);
    
    mesh.skeleton.bones.resize(sourceMesh->mNumBones);
    
    for (unsigned int i = 0; i < mesh.skeleton.bones.size(); ++i)
    {
        const aiBone *sourceBone = sourceMesh->mBones[i];
        
        mesh.skeleton.bones[i] = Bone(std::string(sourceBone->mName.data),
                                      aiMatrixToMat4(sourceBone->mOffsetMatrix));
        
        //Retrieve weights for vertices affected by this bone.
        for (unsigned int j = 0; j < sourceBone->mNumWeights; ++j)
        {
            const unsigned int id = sourceBone->mWeights[j].mVertexId;
            const float weight = sourceBone->mWeights[j].mWeight;
            
            if (id >= mesh.vertices.size() || weight < 0.0f || weight > 1.0f)
            {
                std::cerr << "Mesh has invalid bone indices " << id << " or weights " << weight << "!" << std::endl;
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
                std::cerr << "Warning: mesh contains more than four bones per vertex, discarding excess data!" << std::endl;
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
    
#ifndef NDEBUG
    //Verify bone weights and indices.
    for (unsigned int i = 0; i < mesh.vertices.size(); ++i)
    {
        const int nrBones = mesh.skeleton.bones.size();
        const AnimatedMeshVertex v = mesh.vertices[i];
        const float weightSum = v.weights.x + v.weights.y + v.weights.z + v.weights.w;
        
        if (v.weights.x < 0.0f || v.weights.x > 1.0f ||
            v.weights.y < 0.0f || v.weights.y > 1.0f ||
            v.weights.z < 0.0f || v.weights.z > 1.0f ||
            v.weights.w < 0.0f || v.weights.w > 1.0f ||
            fabsf(weightSum - 1.0f) > 1.0e-2)
        {
            std::cerr << "Warning: invalid weights " << v.weights << " (sum " << weightSum << ")!" << std::endl;
        
            //Normalise weight sum.
            mesh.vertices[i].weights /= weightSum;
        }
        
        if ((v.bones.x < 0 || v.bones.x >= nrBones ||
             v.bones.y < 0 || v.bones.y >= nrBones ||
             v.bones.z < 0 || v.bones.z >= nrBones ||
             v.bones.w < 0 || v.bones.w >= nrBones) && nrBones > 0)
        {
            std::cerr << "Invalid bone indices " << v.bones << "!" << std::endl;
            throw std::exception();
        }
    }
#endif
}

size_t getAiNrAnimationFrames(const aiAnimation *animation)
{
    size_t nrFrames = 0;
    
    for (unsigned int i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim *n = animation->mChannels[i];
        
        nrFrames = std::max<size_t>(n->mNumPositionKeys, nrFrames);
        nrFrames = std::max<size_t>(n->mNumRotationKeys, nrFrames);
        nrFrames = std::max<size_t>(n->mNumScalingKeys, nrFrames);
    }
    
    return nrFrames;
}

void copyAiAnimation(const aiScene *scene, const aiMesh *sourceMesh, const unsigned int &animationIndex,
    std::map<std::string, unsigned int> &boneNameToIndex,
    std::map<std::string, const aiNode *> &nodeNameToPointer,
    std::map<std::string, aiMatrix4x4> &nodeNameToMatrix,
    Skeleton &skeleton)
{
    const aiAnimation *sourceAnimation = scene->mAnimations[animationIndex];
    Animation *animation = &skeleton.animations[animationIndex];
    const size_t nrFrames = getAiNrAnimationFrames(sourceAnimation);
    
    animation->name = std::string(sourceAnimation->mName.data);
    animation->frames.assign(nrFrames*skeleton.bones.size(), KeyFrame());
    
    if (sourceAnimation->mNumChannels != skeleton.bones.size())
    {
        std::cerr << "Warning: animation '" << animation->name << "' has an invalid number of channels (" << sourceAnimation->mNumChannels << " for " << skeleton.bones.size() << " bones)!" << std::endl;
    }
    
    //Store inverse of global transformation.
    aiMatrix4x4 inverseGlobalTransformation = scene->mRootNode->mTransformation;
    
    inverseGlobalTransformation.Inverse();
    
    //Process all frames.
    KeyFrame *frames = &animation->frames[0];
    
    for (size_t frame = 0; frame < nrFrames; ++frame)
    {
        //For this frame, first reset all transformations to their originals.
        for (std::map<std::string, aiMatrix4x4>::iterator i = nodeNameToMatrix.begin(); i != nodeNameToMatrix.end(); ++i)
        {
            assert(nodeNameToPointer[i->first]);
            i->second = nodeNameToPointer[i->first]->mTransformation;
        }
        
        //Then, retrieve all transformations that are stored in the animation data for the corresponding nodes.
        for (size_t i = 0; i < sourceAnimation->mNumChannels; ++i)
        {
            const aiNodeAnim *nodeAnim = sourceAnimation->mChannels[i];
            
            //Get data for this frame.
            aiVector3D scale(1.0f, 1.0f, 1.0f);
            aiQuaternion rotate(1.0f, 0.0f, 0.0f, 0.0f);
            aiVector3D translate(0.0f, 0.0f, 0.0f);
            
            if (frame < nodeAnim->mNumScalingKeys) scale = nodeAnim->mScalingKeys[frame].mValue;
            else if (nodeAnim->mNumScalingKeys > 0) scale = nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mValue;
            if (frame < nodeAnim->mNumRotationKeys) rotate = nodeAnim->mRotationKeys[frame].mValue;
            else if (nodeAnim->mNumRotationKeys > 0) rotate = nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue;
            if (frame < nodeAnim->mNumPositionKeys) translate = nodeAnim->mPositionKeys[frame].mValue;
            else if (nodeAnim->mNumPositionKeys > 0) translate = nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mValue;
            
            //Create transformation matrix.
            if (nodeNameToMatrix.find(nodeAnim->mNodeName.data) == nodeNameToMatrix.end())
            {
                std::cerr << "Warning: animation data for node '" << nodeAnim->mNodeName.data << "' is not available!" << std::endl;
                throw std::exception();
            }
            
            aiMatrix4x4 scaleMatrix;
            aiMatrix4x4 rotationMatrix = aiMatrix4x4(rotate.GetMatrix());
            aiMatrix4x4 translationMatrix;
            
            aiMatrix4x4::Scaling(scale, scaleMatrix);
            aiMatrix4x4::Translation(translate, translationMatrix);
            
            nodeNameToMatrix[nodeAnim->mNodeName.data] = translationMatrix*rotationMatrix*scaleMatrix;
        }
        
        //Recursively update these transformations.
        updateAiNodeMatrices(scene->mRootNode, aiMatrix4x4(), nodeNameToMatrix);

        //Assign the updated transformations to the corresponding bones.
        for (std::map<std::string, aiMatrix4x4>::const_iterator i = nodeNameToMatrix.begin(); i != nodeNameToMatrix.end(); ++i)
        {
            std::map<std::string, unsigned int>::const_iterator boneIterator = boneNameToIndex.find(i->first);
            
            if (boneIterator != boneNameToIndex.end())
            {
                const unsigned int boneIndex = boneIterator->second;
                //const aiMatrix4x4 finalTransformation = inverseGlobalTransformation*i->second*sourceMesh->mBones[boneIndex]->mOffsetMatrix;
                const aiMatrix4x4 finalTransformation = i->second*sourceMesh->mBones[boneIndex]->mOffsetMatrix;
                
                aiVector3D scale(1.0f, 1.0f, 1.0f);
                aiQuaternion rotate(1.0f, 0.0f, 0.0f, 0.0f);
                aiVector3D translate(0.0f, 0.0f, 0.0f);
                
                finalTransformation.Decompose(scale, rotate, translate);
                
                frames[boneIndex] = KeyFrame(vec3(scale.x, scale.y, scale.z),
                                             frame,
                                             quatconj(vec4(rotate.x, rotate.y, rotate.z, rotate.w)),
                                             vec3(translate.x, translate.y, translate.z));
            }
        }
        
        //Advance to next frame.
        frames += skeleton.bones.size();
    }
    
#ifndef NDEBUG
    std::cerr << "Added animation '" << animation->name << "' with " << nrFrames << " frames, resulting in " << animation->frames.size() << " keyframes." << std::endl;
#endif
}

void copyAiAnimations(const aiScene *scene, const aiMesh *sourceMesh, Skeleton &skeleton)
{
    //Create map from bone names to their indices.
    std::map<std::string, unsigned int> boneNameToIndex;
    
    for (unsigned int i = 0; i < skeleton.bones.size(); ++i)
    {
        if (!boneNameToIndex.insert(std::make_pair(skeleton.bones[i].name, i)).second)
        {
            std::cerr << "Bone name '" << skeleton.bones[i].name << "' is not unique!" << std::endl;
            throw std::exception();
        }
    }
    
    assert(boneNameToIndex.size() == skeleton.bones.size());
    
    //Create map from node names to their pointers.
    std::map<std::string, const aiNode *> nodeNameToPointer;
    
    detail::setAiNodePointers(scene->mRootNode, nodeNameToPointer);
    
    //We need to keep track of all transformation matrices.
    std::map<std::string, aiMatrix4x4> nodeNameToMatrix;
    
    for (std::map<std::string, const aiNode *>::const_iterator i = nodeNameToPointer.begin(); i != nodeNameToPointer.end(); ++i)
    {
        nodeNameToMatrix.insert(std::make_pair(i->first, aiMatrix4x4()));
    }
    
    //Process all animations.
    skeleton.animations.resize(scene->mNumAnimations);
    
    for (unsigned int i = 0; i < skeleton.animations.size(); ++i)
    {
        copyAiAnimation(scene, sourceMesh, i, boneNameToIndex, nodeNameToPointer, nodeNameToMatrix, skeleton);
    }
}

} //namespace detail

} //namespace io

} //namespace mesh

} //namespace tiny

AnimatedMesh tiny::mesh::io::readAnimatedMesh(const std::string &fileName, const std::string &meshName)
{
    //Use AssImp to read all data from the file.
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(fileName.c_str(), aiProcessPreset_TargetRealtime_Quality);
    
    if (!scene)
    {
        std::cerr << "Unable to read '" << fileName << "' from disk:" << importer.GetErrorString() << "!" << std::endl;
        throw std::exception();
    }
    
    const aiMesh *sourceMesh = detail::getAiMesh(scene, meshName);
    AnimatedMesh mesh;
    
    assert(sourceMesh);
    detail::copyAiMeshVertices<AnimatedMesh, AnimatedMeshVertex>(sourceMesh, mesh, aiMatrix4x4());
    detail::copyAiMeshIndices(sourceMesh, mesh);
    detail::copyAiMeshBones(sourceMesh, mesh);
    
    //Copy animations if available.
    if (scene->HasAnimations())
    {
        detail::copyAiAnimations(scene, sourceMesh, mesh.skeleton);
    }
    
    std::cerr << "Read mesh '" << meshName << "' with " << mesh.vertices.size() << " vertices, " << mesh.indices.size()/3 << " triangles, " << mesh.skeleton.bones.size() << " bones, and " << mesh.skeleton.animations.size() << " animations from '" << fileName << "'." << std::endl;
    
    //importer will go out of scope, which will free all read data automatically.
    return mesh;
}

