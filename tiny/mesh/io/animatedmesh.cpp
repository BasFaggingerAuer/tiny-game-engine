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

#include <assimp/aiScene.h>
#include <assimp/assimp.hpp>
#include <assimp/aiPostProcess.h>

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
        
        if (v.bones.x < 0 || v.bones.x >= nrBones ||
            v.bones.y < 0 || v.bones.y >= nrBones ||
            v.bones.z < 0 || v.bones.z >= nrBones ||
            v.bones.w < 0 || v.bones.w >= nrBones)
        {
            std::cerr << "Invalid bone indices " << v.bones << "!" << std::endl;
            throw std::exception();
        }
    }
#endif
}

void setAiNodePointers(aiNode *node, std::map<std::string, aiNode *> &nodeNameToPointer)
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

void updateAiNodeMatrices(aiNode *node, const mat4 &transformation, std::map<std::string, mat4> &nodeNameToMatrix)
{
    const mat4 newTransformation = transformation*nodeNameToMatrix[node->mName.data];
    
    nodeNameToMatrix[node->mName.data] = newTransformation;
    
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        updateAiNodeMatrices(node->mChildren[i], newTransformation, nodeNameToMatrix);
    }
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

void copyAiAnimation(const aiScene *scene, const unsigned int &animationIndex,
    std::map<std::string, unsigned int> &boneNameToIndex,
    std::map<std::string, mat4> &nodeNameToMatrix,
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
    
    //Process all frames.
    KeyFrame *frames = &animation->frames[0];
    
    for (size_t frame = 0; frame < nrFrames; ++frame)
    {
        //For this frame, first reset all transformations to the identity.
        for (std::map<std::string, mat4>::iterator i = nodeNameToMatrix.begin(); i != nodeNameToMatrix.end(); ++i)
        {
            i->second = mat4::identityMatrix();
        }
        
        //Then, retrieve all transformations that are stored in the animation data for the corresponding nodes.
        for (size_t i = 0; i < sourceAnimation->mNumChannels; ++i)
        {
            const aiNodeAnim *nodeAnim = sourceAnimation->mChannels[i];
            
            //Get data for this frame.
            //TODO: Take care of scaling.
            //aiVector3D ai_scale(1.0f, 1.0f, 1.0f);
            aiQuaternion ai_rotate(0.0f, 0.0f, 0.0f, 1.0f);
            aiVector3D ai_translate(0.0f, 0.0f, 0.0f);
            
            //if (frame < nodeAnim->mNumScalingKeys) ai_scale = nodeAnim->mScalingKeys[frame].mValue;
            //else if (nodeAnim->mNumScalingKeys > 0) ai_scale = nodeAnim->mScalingKeys[nodeAnim->mNumScalingKeys - 1].mValue;
            if (frame < nodeAnim->mNumRotationKeys) ai_rotate = nodeAnim->mRotationKeys[frame].mValue;
            else if (nodeAnim->mNumRotationKeys > 0) ai_rotate = nodeAnim->mRotationKeys[nodeAnim->mNumRotationKeys - 1].mValue;
            if (frame < nodeAnim->mNumPositionKeys) ai_translate = nodeAnim->mPositionKeys[frame].mValue;
            else if (nodeAnim->mNumPositionKeys > 0) ai_translate = nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mValue;
            
            //Create transformation matrix.
            //const vec3 scale(ai_scale.x, ai_scale.y, ai_scale.z);
            const vec4 rotate(normalize(vec4(ai_rotate.x, ai_rotate.y, ai_rotate.z, ai_rotate.w)));
            const vec3 translate(ai_translate.x, ai_translate.y, ai_translate.z);
            
            if (nodeNameToMatrix.find(nodeAnim->mNodeName.data) != nodeNameToMatrix.end())
            {
                nodeNameToMatrix[nodeAnim->mNodeName.data] = mat4(rotate, translate);
            }
            else
            {
                std::cerr << "Warning: animation data for node '" << nodeAnim->mNodeName.data << "' is not available!" << std::endl;
            }
        }
        
        //Recursively update these transformations.
        updateAiNodeMatrices(scene->mRootNode, mat4::identityMatrix(), nodeNameToMatrix);

        //Assign the updated transformations to the corresponding bones.
        for (size_t i = 0; i < sourceAnimation->mNumChannels; ++i)
        {
            const aiNodeAnim *nodeAnim = sourceAnimation->mChannels[i];
            
            if (nodeNameToMatrix.find(nodeAnim->mNodeName.data) != nodeNameToMatrix.end() &&
                boneNameToIndex.find(nodeAnim->mNodeName.data) != boneNameToIndex.end())
            {
                const unsigned int boneIndex = boneNameToIndex[nodeAnim->mNodeName.data];
                const mat4 boneMatrix = skeleton.bones[boneIndex].meshToBone;
                const mat4 transformation = nodeNameToMatrix[nodeAnim->mNodeName.data]*boneMatrix;
                
                frames[boneIndex] = KeyFrame(vec3(1.0f, 1.0f, 1.0f),
                                             frame,
                                             quatmatrix(transformation),
                                             transformation.getTranslation());
            }
            else
            {
                std::cerr << "Warning: animation data for node or bone '" << nodeAnim->mNodeName.data << "' cannot be updated!" << std::endl;
            }
        }
        
        //Advance to next frame.
        frames += skeleton.bones.size();
    }
    
#ifndef NDEBUG
    std::cerr << "Added animation '" << animation->name << "' with " << nrFrames << " frames." << std::endl;
#endif
}

void copyAiAnimations(const aiScene *scene, Skeleton &skeleton)
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
    std::map<std::string, aiNode *> nodeNameToPointer;
    
    detail::setAiNodePointers(scene->mRootNode, nodeNameToPointer);
    
    //We need to keep track of all transformation matrices.
    std::map<std::string, mat4> nodeNameToMatrix;
    
    for (std::map<std::string, aiNode *>::const_iterator i = nodeNameToPointer.begin(); i != nodeNameToPointer.end(); ++i)
    {
        nodeNameToMatrix.insert(std::make_pair(i->first, mat4::identityMatrix()));
    }
    
    //Process all animations.
    skeleton.animations.resize(scene->mNumAnimations);
    
    for (unsigned int i = 0; i < skeleton.animations.size(); ++i)
    {
        copyAiAnimation(scene, i, boneNameToIndex, nodeNameToMatrix, skeleton);
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
        std::cerr << "Unable to read '" << fileName << "' from disk!" << std::endl;
        throw std::exception();
    }
    
    const aiMesh *sourceMesh = detail::getAiMesh(scene, meshName);
    AnimatedMesh mesh;
    
    assert(sourceMesh);
    detail::copyAiMeshVertices<AnimatedMesh, AnimatedMeshVertex>(sourceMesh, mesh);
    detail::copyAiMeshIndices(sourceMesh, mesh);
    detail::copyAiMeshBones(sourceMesh, mesh);
    
    //Copy animations if available.
    if (scene->HasAnimations())
    {
        detail::copyAiAnimations(scene, mesh.skeleton);
    }
    
    std::cerr << "Read mesh '" << meshName << "' with " << mesh.vertices.size() << " vertices, " << mesh.indices.size()/3 << " triangles, " << mesh.skeleton.bones.size() << " bones, and " << mesh.skeleton.animations.size() << " animations from '" << fileName << "'." << std::endl;
    
    //importer will go out of scope, which will free all read data automatically.
    return mesh;
}

