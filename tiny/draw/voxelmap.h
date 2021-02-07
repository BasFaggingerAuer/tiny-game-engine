/*
Copyright 2020, Bas Fagginger Auer.

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
#include <tiny/draw/screensquare.h>
#include <tiny/draw/texture3d.h>

namespace tiny
{

namespace draw
{

struct VoxelIntersection
{
    VoxelIntersection(const float a_distance, const ivec3 &a_voxelIndices, const ivec3 &a_normal) :
        distance(a_distance),
        voxelIndices(a_voxelIndices),
        normal(a_normal)
    {

    }
    
    float distance;
    ivec3 voxelIndices;
    ivec3 normal;
};

class VoxelMap : public tiny::draw::ScreenFillingSquare
{
    public:
        VoxelMap(const int &, const float & = 1.4f, const float & = 1.0e-6f);
        ~VoxelMap();
        std::string getTypeName() const;
        
        template <typename TextureType>
        VoxelIntersection getIntersection(const TextureType &voxelTexture, const vec3 &a_position, const vec3 &a_direction) const
        {
            //Determine intersection of a single ray with the voxel map on the CPU.
            const vec3 position = a_position + 0.5f*scale*vec3(static_cast<float>(voxelTexture.getWidth()), 0.0f, static_cast<float>(voxelTexture.getDepth()));
            const vec3 direction = normalize(a_direction);
            const vec3 invDirection = 1.0f/max(abs(direction), vec3(epsilon));
            const ivec3 directionSign = to_int((step(vec3(-epsilon), direction) - 1.0f) + step(vec3(epsilon), direction));
            ivec3 voxelIndices = to_int(floor(position/scale));
            float baseDist = 0.0f;

            for (int i = 0; i < 4; ++i)
            {
                vec4 voxel = voxelTexture(voxelIndices.x, voxelIndices.y, voxelIndices.z);
                
                baseDist += std::max(0.0f, 255.0f*voxel.z - epsilon);
                voxelIndices = to_int(floor((position/scale) + baseDist*direction));
                //std::cout << "PRE-STEP: " << baseDist << std::endl;
            }

            vec3 distances = max((step(vec3(0.0f), direction) - fract((position/scale) + baseDist*direction))*to_float(directionSign)*invDirection, vec3(0.0f));
            ivec3 mask = lessThanEqual(distances, min(distances.yzx(), distances.zxy()));
            float dist = baseDist;
            
            for (int i = 0; i < nrSteps; ++i)
            {
                vec4 voxel = voxelTexture(voxelIndices.x, voxelIndices.y, voxelIndices.z);
                
                if (voxel.y == 0.0f)
                {
                    //std::cout << i << " VOXEL ITERATIONS ESCAPE" << std::endl;
                    return VoxelIntersection(-1.0f, ivec3(0), ivec3(0));
                }

                if (voxel.x > 0.0f)
                {
                    //std::cout << i << " VOXEL ITERATIONS" << std::endl;
                    return VoxelIntersection(dist*scale, voxelIndices, -directionSign*mask);
                }
                
                mask = lessThanEqual(distances, min(distances.yzx(), distances.zxy()));
                dist = baseDist + dot(to_float(mask), distances);
                distances += to_float(mask)*invDirection;
                voxelIndices += mask*directionSign;
            }
            
            //std::cout << nrSteps << " VOXEL ITERATIONS FULL ESCAPE" << std::endl;
            return VoxelIntersection(-1.0f, ivec3(0), ivec3(0));
        }

        template <typename TextureType>
        static void clearDistanceMap(TextureType &voxelTexture)
        {
            //Set distance map to 0 to effectively remove it.
            const size_t width = voxelTexture.getWidth(), height = voxelTexture.getHeight(), depth = voxelTexture.getDepth(), channels = voxelTexture.getChannels();

            if (channels < 3)
            {
                std::cerr << "Error: Need a texture with at least 3 channels to clear the voxel distance map!" << std::endl;
                assert(false);
                return;
            }

            for (size_t z = 0; z < depth; ++z)
            {
                for (size_t y = 0; y < height; ++y)
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        voxelTexture[channels*(width*height*z + width*y + x) + 2u] = 0;
                    }
                }
            }

            voxelTexture.sendToDevice();
        }

        template <typename TextureType>
        static void setDistanceMap(TextureType &voxelTexture)
        {
            //Store the dimensions of the largest empty cube surrounding each voxel in a given voxel texture (in Z).
            //Used to accelerate voxel rendering.
            const size_t width = voxelTexture.getWidth(), height = voxelTexture.getHeight(), depth = voxelTexture.getDepth(), channels = voxelTexture.getChannels();

            std::cerr << "Creating distance table for " << width << "x" << height << "x" << depth << " voxels..." << std::endl;

            if (channels < 3)
            {
                std::cerr << "Error: Need a texture with at least 3 channels to store the voxel distance map!" << std::endl;
                assert(false);
                return;
            }

            auto sat = VoxelMap::createSummedAreaTable(voxelTexture);
            
            for (size_t z = 0; z < depth; ++z)
            {
                for (size_t y = 0; y < height; ++y)
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        if (voxelTexture(x, y, z).x > 0.0f)
                        {
                            //Are inside a voxel --> distance 0.
                            voxelTexture[channels*(width*height*z + width*y + x) + 2u] = 0;
                        }
                        else
                        {
                            //Are not inside a voxel, perform bisection.
                            int shift = 7;
                            int size = 0;
                    
                            while (shift >= 0)
                            {
                                while (shift >= 0 && VoxelMap::getNrVoxelsInCube(voxelTexture, sat, x, y, z, size + (1 << shift)) > 0)
                                {
                                    shift--;
                                }
                        
                                if (shift >= 0)
                                {
                                    size += (1 << shift);
                                    shift--;
                                }
                            }
                    
#ifndef NDEBUG
                            assert(VoxelMap::getNrVoxelsInCube(voxelTexture, sat, x, y, z, size) == 0);
                            assert(size >= 0 && size <= 255);
                    
                            if (size < 255) {
                                assert(VoxelMap::getNrVoxelsInCube(voxelTexture, sat, x, y, z, size + 1) != 0);
                            }
#endif
                            voxelTexture[channels*(width*height*z + width*y + x) + 2u] = size;
                        }
                    }
                }
            }

            voxelTexture.sendToDevice();
        }
        
        template <typename TextureType>
        void setVoxelMap(const TextureType &voxelTexture, const float &scale_)
        {
            uniformMap.setTexture(voxelTexture, "voxelTexture");
            
            scale = scale_;
            uniformMap.setFloatUniform(scale, "voxelScale");
            uniformMap.setVec3Uniform(static_cast<float>(voxelTexture.getWidth()), static_cast<float>(voxelTexture.getHeight()), static_cast<float>(voxelTexture.getDepth()), "voxelTextureSize");
        }
        
        template <typename TextureType>
        void setCubeMaps(const TextureType &cubemapTextureArray)
        {
            uniformMap.setTexture(cubemapTextureArray, "cubemapTextureArray");
        }
        
        float getScale() const;
        
        std::string getFragmentShaderCode() const;
        
    private:
        template <typename TextureType>
        static std::vector<size_t> createSummedAreaTable(const TextureType &voxelTexture)
        {
            const size_t width = voxelTexture.getWidth(), height = voxelTexture.getHeight(), depth = voxelTexture.getDepth();

            std::cerr << "Creating summed-area table with voxel counts for " << width << "x" << height << "x" << depth << " voxels..." << std::endl;
    
            std::vector<size_t> summedAreaTable(width*height*depth, 0);
            size_t *out = summedAreaTable.data();
    
            for (size_t z = 0; z < depth; ++z)
            {
                for (size_t y = 0; y < height; ++y)
                {
                    for (size_t x = 0; x < width; ++x)
                    {
                        *out++ = (voxelTexture(x, y, z).x > 0.0f ? 1u : 0u)
                                 + (x < 1 ? 0 : summedAreaTable[(x - 1) + y*width + z*width*height])
                                 + (y < 1 ? 0 : summedAreaTable[x + (y - 1)*width + z*width*height])
                                 + (z < 1 ? 0 : summedAreaTable[x + y*width + (z - 1)*width*height])
                                 - (x < 1 || y < 1 ? 0 : summedAreaTable[(x - 1) + (y - 1)*width + z*width*height])
                                 - (x < 1 || z < 1 ? 0 : summedAreaTable[(x - 1) + y*width + (z - 1)*width*height])
                                 - (y < 1 || z < 1 ? 0 : summedAreaTable[x + (y - 1)*width + (z - 1)*width*height])
                                 + (x < 1 || y < 1 || z < 1 ? 0 : summedAreaTable[(x - 1) + (y - 1)*width + (z - 1)*width*height]);
                    }
                }
            }
    
            //Check that we counted correctly.
            assert(static_cast<size_t>(std::distance(summedAreaTable.data(), out)) == width*height*depth);
            
#ifndef NDEBUG
            //Check that the total count agrees.
            if (true) {
                size_t count = 0;
        
                for (size_t z = 0; z < depth; ++z)
                {
                    for (size_t y = 0; y < height; ++y)
                    {
                        for (size_t x = 0; x < width; ++x)
                        {
                            count += (voxelTexture(x, y, z).x > 0.0f ? 1u : 0u);
                        }
                    }
                }
                
                assert(count == summedAreaTable.back());
            }
#endif
    
            std::cerr << summedAreaTable.back() << "/" << summedAreaTable.size() << " (" << (100LL*summedAreaTable.back())/summedAreaTable.size() << "%) voxels are nonempty." << std::endl;
    
            return summedAreaTable;
        }

        template <typename TextureType>
        static size_t getNrVoxelsInCube(const TextureType &voxelTexture, const std::vector<size_t> summedAreaTable, const int64_t &x, const int64_t &y, const int64_t &z, const int64_t &size)
        {
            const int64_t width = voxelTexture.getWidth(), height = voxelTexture.getHeight(), depth = voxelTexture.getDepth();

            if (static_cast<size_t>(width*height*depth) != summedAreaTable.size())
            {
                std::cerr << "Error: Incompatible 3D texture size and summed area table size!" << std::endl;
                assert(false);
                return 0;
            }

            //Ensure we sample within the array.
            const int64_t x0 = x - size - 1;
            const int64_t x1 = (x < width - size  ? x + size     : width - 1);
            const int64_t y0 = y - size - 1;
            const int64_t y1 = (y < height - size ? y + size     : height - 1);
            const int64_t z0 = z - size - 1;
            const int64_t z1 = (z < depth - size  ? z + size     : depth - 1);
    
            assert(size >= 0);
    
            //Have to handle xyz0 < 0 gracefully because we store the <= counts, not < counts in voxel_counts.
            return (summedAreaTable[static_cast<size_t>(x1 + width*y1 + width*height*z1)]
                    + (y0 >= 0 && z0 >= 0 ? summedAreaTable[static_cast<size_t>(x1 + width*y0 + width*height*z0)] : 0u)
                    + (x0 >= 0 && z0 >= 0 ? summedAreaTable[static_cast<size_t>(x0 + width*y1 + width*height*z0)] : 0u)
                    + (x0 >= 0 && y0 >= 0 ? summedAreaTable[static_cast<size_t>(x0 + width*y0 + width*height*z1)] : 0u)) -
                   ((x0 >= 0 && y0 >= 0 && z0 >= 0 ? summedAreaTable[static_cast<size_t>(x0 + width*y0 + width*height*z0)] : 0u)
                    + (x0 >= 0 ? summedAreaTable[static_cast<size_t>(x0 + width*y1 + width*height*z1)] : 0u)
                    + (y0 >= 0 ? summedAreaTable[static_cast<size_t>(x1 + width*y0 + width*height*z1)] : 0u)
                    + (z0 >= 0 ? summedAreaTable[static_cast<size_t>(x1 + width*y1 + width*height*z0)] : 0u));
        }
        
        const int nrSteps;
        const float lodScale;
        const float epsilon;
        float scale;
};

}

}
