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

#include <string>
#include <vector>

#include <tiny/math/vec.h>

namespace tiny
{

namespace mesh
{

struct KeyFrame
{
    KeyFrame() :
        scaleAndTime(1.0f, 1.0f, 1.0f, 0.0f),
        rotate(0.0f, 0.0f, 0.0f, 1.0f),
        translate(0.0f, 0.0f, 0.0f, 0.0f)
    {

    }
    
    KeyFrame(const vec3 &a_scale,
             const float &a_time,
             const vec4 &a_rotate,
             const vec4 &a_translate) :
             scaleAndTime(vec4(a_scale.x, a_scale.y, a_scale.z, a_time)),
             rotate(normalize(a_rotate)),
             translate(a_translate)
    {

    }
    
    vec4 scaleAndTime;
    vec4 rotate;
    vec4 translate;
};

struct Bone
{
    Bone() :
        name(""),
        meshToBone(mat4::identityMatrix())
    {

    }
    
    Bone(const std::string &a_name, const mat4 &a_meshToBone) :
        name(a_name),
        meshToBone(a_meshToBone)
    {

    }
    
    std::string name;
    mat4 meshToBone;
};

struct AnimatedMeshVertex
{
    AnimatedMeshVertex() :
        textureCoordinate(0.0f, 0.0f),
        normal(0.0f, 0.0f, 1.0f),
        position(0.0f, 0.0f, 0.0f),
        weights(1.0f, 0.0f, 0.0f, 0.0f),
        bones(0, 0, 0, 0)
    {
        
    }
    
    AnimatedMeshVertex(const vec2 &a_textureCoordinate,
                     const vec3 &a_tangent,
                     const vec3 &a_normal,
                     const vec3 &a_position) :
        textureCoordinate(a_textureCoordinate),
        tangent(a_tangent),
        normal(a_normal),
        position(a_position),
        weights(vec4(1.0f, 0.0f, 0.0f, 0.0f)),
        bones(ivec4(0, 0, 0, 0))
    {

    }

    AnimatedMeshVertex(const vec2 &a_textureCoordinate,
                     const vec3 &a_tangent,
                     const vec3 &a_normal,
                     const vec3 &a_position,
                     const vec4 &a_weights,
                     const ivec4 &a_bones) :
        textureCoordinate(a_textureCoordinate),
        tangent(a_tangent),
        normal(a_normal),
        position(a_position),
        weights(a_weights),
        bones(a_bones)
    {

    }

    vec2 textureCoordinate;
    vec3 tangent;
    vec3 normal;
    vec3 position;
    vec4 weights;
    ivec4 bones;
};

class Animation
{
    public:
        Animation();
        ~Animation();

        std::string name;
        std::vector<KeyFrame> frames;
};

class Skeleton
{
    public:
        Skeleton();
        ~Skeleton();
        
        std::vector<Bone> bones;
        std::vector<Animation> animations;
};

class AnimatedMesh
{
    public:
        AnimatedMesh();
        ~AnimatedMesh();
        
        Skeleton skeleton;
        std::vector<AnimatedMeshVertex> vertices;
        std::vector<unsigned int> indices;
};

}

}

