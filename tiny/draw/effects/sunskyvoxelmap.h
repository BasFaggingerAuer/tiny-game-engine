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
#include <vector>

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/screensquare.h>

namespace tiny
{

namespace draw
{

namespace effects
{

class SunSkyVoxelMap : public tiny::draw::ScreenFillingSquare
{
    public:
        SunSkyVoxelMap(const int &, const float & = 1.0e-3f);
        ~SunSkyVoxelMap();
        std::string getTypeName() const;
        
        std::string getFragmentShaderCode() const;
        
        template <typename TextureType>
        void setSkyTexture(const TextureType &texture)
        {
            const size_t width = texture.getWidth();
            const size_t height = texture.getHeight();
                
            if (width > 0)
            {
                skyColours.resize(width);
                
                for (size_t i = 0; i < width; ++i)
                {
                    const vec4 colour = texture(i, height - 1);
                    
                    skyColours[i] = vec3(colour.x, colour.y, colour.z);
                }
            }
            
            setSun(sun);
            uniformMap.setTexture(texture, "skyTexture");
        }
        
        template <typename TextureType>
        void setVoxelMap(const TextureType &voxelTexture, const float &scale_)
        {
            uniformMap.setTexture(voxelTexture, "voxelTexture");
            uniformMap.setFloatUniform(scale_, "voxelScale");
            uniformMap.setVec3Uniform(static_cast<float>(voxelTexture.getWidth()), static_cast<float>(voxelTexture.getHeight()), static_cast<float>(voxelTexture.getDepth()), "voxelTextureSize");
        }
        
        void setSun(const vec3 &);
        void setFog(const float &);
        
    private:
        const int nrSteps;
        const float epsilon;
        vec3 sun;
        std::vector<vec3> skyColours;
};

}

}

}

