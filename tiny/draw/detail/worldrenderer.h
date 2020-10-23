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
#include <list>

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/rendererwithcamera.h>

namespace tiny
{

namespace draw
{

namespace detail
{

//The first stage of world rendering: geometry --> diffuse, normals, positions, and depth.
class WorldRendererStageOne : public RendererWithCamera
{
    public:
        WorldRendererStageOne(const float &, const int &, const int &);
        virtual ~WorldRendererStageOne();
        
        template<typename T, size_t Channels>
        void setDiffuseTarget(const Texture2D<T, Channels> &texture)
        {
            setTextureTarget(texture, "diffuse");
            uniformMap.setVec2Uniform(vec2(1.0f/static_cast<float>(texture.getWidth()),
                                           1.0f/static_cast<float>(texture.getHeight())),
                                      "inverseScreenSize");
        }
        
        template<typename T, size_t Channels>
        void setNormalsTarget(const Texture2D<T, Channels> &texture)
        {
            setTextureTarget(texture, "worldNormal");
        }
        
        template<typename T, size_t Channels>
        void setPositionsTarget(const Texture2D<T, Channels> &texture)
        {
            setTextureTarget(texture, "worldPosition");
        }
};

//The second stage of world rendering: diffuse, normals, positions, and depth --> colour.
class WorldRendererStageTwo : public RendererWithCamera
{
    public:
        WorldRendererStageTwo(const float &, const int &, const int &);
        virtual ~WorldRendererStageTwo();
        
        void setScreenSize(const int &, const int &);

        template <typename TextureType>
        void setDiffuseSource(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "diffuseTexture");
        }
        
        template <typename TextureType>
        void setNormalsSource(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "worldNormalTexture");
        }
        
        template <typename TextureType>
        void setPositionsSource(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "worldPositionTexture");
        }
};

}

}

}

