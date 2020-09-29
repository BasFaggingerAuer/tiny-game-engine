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
#include <iostream>
#include <algorithm>

#include <tiny/img/io/image.h>

#include "voxel.h"

using namespace rpg;
using namespace tiny;

GameVoxelMap::GameVoxelMap(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading voxel map resources..." << std::endl;
    
    assert(std::string(el->Value()) == "voxelmap");
    
    int width = 64;
    int height = 64;
    int depth = 64;
    float voxelSize = 1.0f;
    
    el->QueryIntAttribute("width", &width);
    el->QueryIntAttribute("height", &height);
    el->QueryIntAttribute("depth", &depth);
    el->QueryFloatAttribute("scale", &voxelSize);
    
    //Read cubemaps.
    std::vector<img::Image> cubeMaps;
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (std::string(sl->Value()) == "cubemap")
        {
            std::string textureFileName = "";
            
            sl->QueryStringAttribute("all", &textureFileName);
            
            if (!textureFileName.empty())
            {
                auto texture = img::io::readImage(path + textureFileName);
                
                for (int im = 0; im < 6; ++im)
                {
                    cubeMaps.push_back(texture);
                }
            }
            else
            {
                sl->QueryStringAttribute("px", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mx", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("py", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("my", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("pz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
            }
        }
        else
        {
            std::cerr << "Warning: unknown data " << sl->Value() << " encountered in XML!" << std::endl;
        }
    }
    
    voxelCubeArrayTexture = new draw::RGBTexture2DCubeArray(cubeMaps.begin(), cubeMaps.end(), draw::tf::repeat | draw::tf::mipmap);
    voxelMap = new draw::VoxelMap(std::max(width, std::max(height, depth))*4);
    voxelTexture = new draw::RGTexture3D(width, height, depth, draw::tf::none);
    
    //Create checkerboard pattern.
    for (size_t z = 0; z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 0; y < voxelTexture->getHeight(); ++y)
        {
            for (size_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 1] = (((x ^ y ^ z) & 1) == 0 ? 116 : 140);
            }
        }
    }
    
    voxelTexture->sendToDevice();
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    voxelMap->setCubeMaps(*voxelCubeArrayTexture);
    setVoxelBasePlane(1);
    createVoxelPalette();
}

GameVoxelMap::~GameVoxelMap()
{
    delete voxelMap;
    delete voxelTexture;
    delete voxelCubeArrayTexture;
}

void GameVoxelMap::createVoxelPalette()
{
    //Create palette for different voxel types.
    const size_t n = voxelCubeArrayTexture->getDepth()/6;
    const size_t p = static_cast<size_t>(ceil(sqrtf(static_cast<float>(n))));
    size_t count = 0;
    
    for (size_t z = (voxelTexture->getDepth() - p)/2; z <= (voxelTexture->getDepth() + p)/2 && z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 1; y < 2; ++y)
        {
            for (size_t x = 0; x < p && x < voxelTexture->getWidth() && count < n; ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = 1 + count++;
            }
        }
    }
    
    voxelTexture->sendToDevice();
}

void GameVoxelMap::setVoxelBasePlane(const int &v)
{
    //Set all voxels at y = 0 to given type.
    for (size_t z = 0; z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 0; y < 1; ++y)
        {
            for (size_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = v;
            }
        }
    }
    
    voxelTexture->sendToDevice();
}

void GameVoxelMap::setVoxel(const ivec3 &p, const int &v)
{
    (*voxelTexture)[voxelTexture->getChannels()*(clamp<int>(p.z + voxelTexture->getDepth()/2, 0, voxelTexture->getDepth() - 1)*voxelTexture->getHeight()*voxelTexture->getWidth() + clamp<int>(p.y, 0, voxelTexture->getHeight() - 1)*voxelTexture->getWidth() + clamp<int>(p.x + voxelTexture->getWidth()/2, 0, voxelTexture->getWidth() - 1)) + 0] = v;
    
    voxelTexture->sendToDevice();
}

int GameVoxelMap::getVoxel(const ivec3 &p) const
{
    return (*voxelTexture)[voxelTexture->getChannels()*(clamp<int>(p.z + voxelTexture->getDepth()/2, 0, voxelTexture->getDepth() - 1)*voxelTexture->getHeight()*voxelTexture->getWidth() + clamp<int>(p.y, 0, voxelTexture->getHeight() - 1)*voxelTexture->getWidth() + clamp<int>(p.x + voxelTexture->getWidth()/2, 0, voxelTexture->getWidth() - 1)) + 0];
}

float GameVoxelMap::getScale() const
{
    return voxelMap->getScale();
}

float GameVoxelMap::getScaledWidth() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getWidth());
}

float GameVoxelMap::getScaledHeight() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getHeight());
}

float GameVoxelMap::getScaledDepth() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getDepth());
}

draw::VoxelIntersection GameVoxelMap::getIntersection(const vec3 &a_position, const vec3 &a_direction) const
{
    draw::VoxelIntersection intersection = voxelMap->getIntersection(*voxelTexture, a_position, a_direction);
    
    //Center voxel map.
    intersection.voxelIndices -= ivec3(voxelTexture->getWidth()/2, 0, voxelTexture->getDepth()/2);
    
    return intersection;
}

int GameVoxelMap::getBaseHeight(const int &x, const int &z) const
{
    //Determine height above voxel map.
    int baseHeight;
    
    for (baseHeight = 0; baseHeight < static_cast<int>(voxelTexture->getHeight()); ++baseHeight)
    {
        if ((*voxelTexture)(x + voxelTexture->getWidth()/2,
                            baseHeight,
                            z + voxelTexture->getDepth()/2).x == 0.0f)
        {
            break;
        }
    }
    
    return baseHeight;
}

