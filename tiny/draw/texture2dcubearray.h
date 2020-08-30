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

#include <iostream>
#include <exception>

#include <tiny/draw/texture.h>
#include <tiny/img/image.h>

namespace tiny
{

namespace draw
{

template<typename T, size_t Channels>
class Texture2DCubeArray : public Texture<T, Channels>
{
    public:
        Texture2DCubeArray(const size_t &a_width, const size_t &a_height, const size_t &a_depth, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_CUBE_MAP_ARRAY, a_flags, a_width, a_height, a_depth)
        {
            if ((a_depth % 6) != 0)
            {
                std::cerr << "You need to supply a number of textures that is a multiple of 6 for a cube texture array!" << std::endl;
                throw std::exception();
            }
        }
        
        Texture2DCubeArray(const Texture2DCubeArray<T, Channels> &texture) :
            Texture<T, Channels>(texture)
        {
            
        }
        
        template<typename ImageIterator>
        Texture2DCubeArray(ImageIterator first, ImageIterator last, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_CUBE_MAP_ARRAY, a_flags, first->width, first->height, last - first)
        {
            size_t offset = 0;
            
            if (((last - first) % 6) != 0)
            {
                std::cerr << "You need to supply a number of textures that is a multiple of 6 for a cube texture array!" << std::endl;
                throw std::exception();
            }
            
            for (ImageIterator i = first; i != last; ++i)
            {
                if (i->height != this->height || i->width != this->width)
                {
                    std::cerr << "All images in a texture array should have the same dimensions!" << std::endl;
                    throw std::exception();
                }
                
                for (size_t y = 0; y < this->height; ++y)
                {
                    for (size_t x = 0; x < this->width; ++x)
                    {
                        for (size_t j = 0; j < Channels; ++j) this->hostData[offset + Channels*(x + this->width*y) + j] = i->data[4*(x + this->width*y) + j];
                    }
                }
                
                offset += Channels*this->width*this->height;
            }
            
            this->sendToDevice();
            
            std::cerr << "Created a " << this->width << "x" << this->height << " cubemap texture array containing " << this->depth/6 << " cubemaps." << std::endl;
        }
        
        ~Texture2DCubeArray()
        {
            
        }
};

typedef Texture2DCubeArray<unsigned char, 3> RGBTexture2DCubeArray;
typedef Texture2DCubeArray<unsigned char, 4> RGBATexture2DCubeArray;

}

}

