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
        VoxelMap(const int &, const float & = 1.0e-6f);
        ~VoxelMap();
        
        template <typename TextureType>
        VoxelIntersection getIntersection(const TextureType &voxelTexture, const vec3 &a_position, const vec3 &a_direction) const
        {
            //Determine intersection of a single ray with the voxel map on the CPU.
            const vec3 position = a_position + 0.5f*scale*vec3(voxelTexture.getWidth(), 0.0f, voxelTexture.getDepth());
            const vec3 direction = normalize(a_direction);
            const vec3 invDirection = 1.0f/max(abs(direction), vec3(epsilon));
            const ivec3 directionSign = to_int((step(vec3(-epsilon), direction) - 1.0f) + step(vec3(epsilon), direction));
            ivec3 voxelIndices = to_int(floor(position/scale));
            vec3 distances = max((step(vec3(0.0f), direction) - fract(position/scale))*to_float(directionSign)*invDirection, vec3(0.0f));
            ivec3 mask = lessThanEqual(distances, min(distances.yzx(), distances.zxy()));
            float dist = 0.0f;
            
            for (int i = 0; i < nrSteps; ++i)
            {
                vec4 voxel = voxelTexture(voxelIndices.x, voxelIndices.y, voxelIndices.z);
                
                if (voxel.x > 0.0f)
                {
                    return VoxelIntersection(dist*scale, voxelIndices, -directionSign*mask);
                }
                
                mask = lessThanEqual(distances, min(distances.yzx(), distances.zxy()));
                dist = dot(to_float(mask), distances);
                distances += to_float(mask)*invDirection;
                voxelIndices += mask*directionSign;
            }
            
            return VoxelIntersection(-1.0f, ivec3(0), ivec3(0));
        }
        
        template <typename TextureType>
        void setVoxelMap(const TextureType &voxelTexture, const float &scale_)
        {
            uniformMap.setTexture(voxelTexture, "voxelTexture");
            
            scale = scale_;
            uniformMap.setFloatUniform(scale, "voxelScale");
            uniformMap.setVec3Uniform(voxelTexture.getWidth(), voxelTexture.getHeight(), voxelTexture.getDepth(), "voxelTextureSize");
        }
        
        template <typename TextureType>
        void setCubeMaps(const TextureType &cubemapTextureArray)
        {
            uniformMap.setTexture(cubemapTextureArray, "cubemapTextureArray");
        }
        
        std::string getFragmentShaderCode() const;
        
    private:
        const int nrSteps;
        const float epsilon;
        float scale;
};

}

}
