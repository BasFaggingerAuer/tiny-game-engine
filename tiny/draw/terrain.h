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

#include <vector>

#include <tiny/math/vec.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/vertexbufferinterpreter.h>

namespace tiny
{

namespace draw
{

namespace detail
{

struct TerrainBlockInstance
{
    TerrainBlockInstance() :
        scaleAndTranslate(vec4(1.0f, 1.0f, 0.0f, 0.0f))
    {

    }

    TerrainBlockInstance(const vec4 &a_scaleAndTranslate) :
        scaleAndTranslate(a_scaleAndTranslate)
    {

    }
    
    vec4 scaleAndTranslate;
};

class TerrainBlockInstanceBufferInterpreter : public VertexBufferInterpreter<TerrainBlockInstance>
{
    public:
        TerrainBlockInstanceBufferInterpreter(const size_t &);
        ~TerrainBlockInstanceBufferInterpreter();
};

class TerrainBlockVertexBufferInterpreter : public VertexBufferInterpreter<vec2>
{
    public:
        TerrainBlockVertexBufferInterpreter(const size_t &, const size_t &);
        ~TerrainBlockVertexBufferInterpreter();
};

class TerrainBlockIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        TerrainBlockIndexBuffer(const size_t &, const size_t &);
        ~TerrainBlockIndexBuffer();
};

class TerrainBlock
{
    public:
        TerrainBlock(const size_t &, const size_t &, const size_t &);
        ~TerrainBlock();
        
        TerrainBlockInstance & operator [] (const size_t &);
        const TerrainBlockInstance & operator [] (const size_t &) const;
        void sendToDevice() const;
        
        void bind(const ShaderProgram &) const;
        void unbind(const ShaderProgram &) const;
        
        TerrainBlockVertexBufferInterpreter vertices;
        TerrainBlockInstanceBufferInterpreter instances;
        TerrainBlockIndexBuffer indices;
};

class TerrainStitchVertexBufferInterpreter : public VertexBufferInterpreter<vec2>
{
    public:
        TerrainStitchVertexBufferInterpreter(const size_t &);
        ~TerrainStitchVertexBufferInterpreter();
};

class TerrainStitchIndexBuffer : public IndexBuffer<unsigned int>
{
    public:
        TerrainStitchIndexBuffer(const size_t &);
        ~TerrainStitchIndexBuffer();
};

class TerrainStitch
{
    public:
        TerrainStitch(const size_t &, const size_t &);
        ~TerrainStitch();
        
        TerrainBlockInstance & operator [] (const size_t &);
        const TerrainBlockInstance & operator [] (const size_t &) const;
        void sendToDevice() const;
        
        void bind(const ShaderProgram &) const;
        void unbind(const ShaderProgram &) const;
        
        TerrainStitchVertexBufferInterpreter vertices;
        TerrainBlockInstanceBufferInterpreter instances;
        TerrainStitchIndexBuffer indices;
};

}

class Terrain : public Renderable
{
    public:
        Terrain(const int &, const int &);
        ~Terrain();
        
        template <typename TextureType1, typename TextureType2, typename TextureType3>
        void setTextures(const TextureType1 &heightTexture, const TextureType2 &normalTexture, const TextureType3 &diffuseTexture, const vec3 &scale_)
        {
            uniformMap.setTexture(heightTexture, "heightTexture");
            uniformMap.setTexture(heightTexture, "farHeightTexture");
            uniformMap.setTexture(normalTexture, "normalTexture");
            uniformMap.setTexture(normalTexture, "farNormalTexture");
            uniformMap.setTexture(diffuseTexture, "diffuseTexture");
            
            scale = scale_;
            farScale = ivec2(1, 1);
            uniformMap.setVec3Uniform(scale, "worldScale");
            uniformMap.setVec4Uniform(1.0f, 1.0f, 0.0f, 0.0f, "scaleAndTranslateFar");
            uniformMap.setVec2Uniform(1.0f/static_cast<float>(heightTexture.getWidth()), 1.0f/static_cast<float>(heightTexture.getHeight()), "inverseHeightTextureSize");
            uniformMap.setVec2Uniform(heightTexture.getWidth()/2, heightTexture.getHeight()/2, "textureShift");
        }
        
        template <typename TextureType1, typename TextureType2, typename TextureType3>
        void setFarTextures(const TextureType1 &heightTexture, const TextureType1 &farHeightTexture,
                            const TextureType2 &normalTexture, const TextureType2 &farNormalTexture,
                            const TextureType3 &diffuseTexture,
                            const vec3 &scale_, const ivec2 &farScale_, const vec2 farOffset)
        {
            uniformMap.setTexture(heightTexture, "heightTexture");
            uniformMap.setTexture(farHeightTexture, "farHeightTexture");
            uniformMap.setTexture(normalTexture, "normalTexture");
            uniformMap.setTexture(farNormalTexture, "farNormalTexture");
            uniformMap.setTexture(diffuseTexture, "diffuseTexture");
            
            scale = scale_;
            farScale = farScale_;
            uniformMap.setVec3Uniform(scale, "worldScale");
            uniformMap.setVec4Uniform(1.0f/static_cast<float>(farScale.x), 1.0f/static_cast<float>(farScale.y), farOffset.x*farHeightTexture.getWidth(), farOffset.y*farHeightTexture.getHeight(), "scaleAndTranslateFar");
            uniformMap.setVec2Uniform(1.0f/static_cast<float>(heightTexture.getWidth()), 1.0f/static_cast<float>(heightTexture.getHeight()), "inverseHeightTextureSize");
            uniformMap.setVec2Uniform(heightTexture.getWidth()/2, heightTexture.getHeight()/2, "textureShift");
        }
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
        void setCameraPosition(const vec3 &);
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        bool updateBlockTranslations(const vec2 &);
        
        int minLevel;
        const int maxLevel;
        const size_t blockSize;
        const size_t superBlockSize;
        vec3 scale;
        ivec2 farScale;
        ivec2 bitShifts;
        std::vector<ivec2> blockTranslations;
        
        detail::TerrainBlock smallBlock;
        detail::TerrainBlock largeBlock;
        detail::TerrainBlock crossBlockX;
        detail::TerrainBlock crossBlockY;
        detail::TerrainBlock ellBlockX;
        detail::TerrainBlock ellBlockY;
        detail::TerrainStitch stitch;
        
        size_t nrSmallBlocks;
        size_t nrCrossBlocks;
        size_t nrLargeBlocks;
        size_t nrEllBlocks;
        size_t nrStitch;
};

}

}
