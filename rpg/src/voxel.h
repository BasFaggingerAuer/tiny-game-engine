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

#include <string>

#include <tinyxml.h>

#include <tiny/math/vec.h>
#include <tiny/draw/texture3d.h>
#include <tiny/draw/texture2dcubearray.h>
#include <tiny/draw/voxelmap.h>

namespace rpg
{

class GameVoxelMap
{
    public:
        GameVoxelMap(const std::string &, TiXmlElement *);
        ~GameVoxelMap();
        
        void createVoxelPalette();
        void createFromCompressedVoxels(const std::string &);
        void setVoxelBasePlane(const int &);
        void setVoxel(const tiny::ivec3 &, const int &);
        int getVoxel(const tiny::ivec3 &) const;
        float getScale() const;
        float getScaledWidth() const;
        float getScaledHeight() const;
        float getScaledDepth() const;
        tiny::draw::VoxelIntersection getIntersection(const tiny::vec3 &, const tiny::vec3 &) const;
        int getBaseHeight(const int &, const int &) const;
        std::string getCompressedVoxels() const;
        
        tiny::draw::VoxelMap *voxelMap;
        tiny::draw::RGTexture3D *voxelTexture;
        
    private:
        tiny::draw::RGBTexture2DCubeArray *voxelCubeArrayTexture;
};

} //namespace rpg



