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

#include <cassert>

#include <tiny/mesh/animatedmesh.h>
#include <tiny/draw/texturebuffer.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/renderable.h>

namespace tiny
{

namespace draw
{

class AnimatedMeshVertexBufferInterpreter : public VertexBufferInterpreter<tiny::mesh::AnimatedMeshVertex>
{
    public:
        AnimatedMeshVertexBufferInterpreter(const tiny::mesh::AnimatedMesh &);
        ~AnimatedMeshVertexBufferInterpreter();
};

class AnimatedMeshIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        AnimatedMeshIndexBuffer(const tiny::mesh::AnimatedMesh &);
        ~AnimatedMeshIndexBuffer();
};

class AnimationTextureBuffer : public Vec4TextureBuffer
{
    public:
        AnimationTextureBuffer();
        ~AnimationTextureBuffer();
        
        template <typename Iterator>
        void setAnimations(Iterator first, Iterator last)
        {
            this->unbindBuffer();
            
            //Count total number of frames.
            size_t nrFrames = 0;
            
            for (Iterator i = first; i != last; ++i)
            {
                nrFrames += i->frames.size();
            }
            
            //Copy all frames to the vertex buffer.
            keyFrameBuffer.resize(nrFrames);
            nrFrames = 0;
            
            for (Iterator i = first; i != last; ++i)
            {
                for (size_t j = 0; j < i->frames.size(); ++j)
                {
                    keyFrameBuffer[nrFrames++] = i->frames[j];
                }
            }
            
            //Send frames to device.
            keyFrameBuffer.sendToDevice();
            this->bindBuffer(keyFrameBuffer);
        }
        
    private:
        VertexBuffer<tiny::mesh::KeyFrame> keyFrameBuffer;
};

class AnimatedMesh : public Renderable
{
    public:
        AnimatedMesh(const tiny::mesh::AnimatedMesh &);
        ~AnimatedMesh();
        
        template <typename TextureType>
        void setAnimationTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "animationTexture");
        }
        
        template <typename TextureType>
        void setDiffuseTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "diffuseTexture");
        }
        
        template <typename TextureType>
        void setNormalTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "normalTexture");
        }
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        AnimatedMeshIndexBuffer indices;
        AnimatedMeshVertexBufferInterpreter vertices;
};

}

}

