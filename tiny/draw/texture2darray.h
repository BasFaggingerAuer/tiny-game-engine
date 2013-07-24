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

#include <tiny/draw/texture.h>
#include <tiny/img/image.h>

namespace tiny
{

namespace draw
{

template<typename T, size_t Channels>
class Texture2DArray : public Texture<T, Channels>
{
    public:
        Texture2DArray(const size_t &a_width, const size_t &a_height, const size_t &a_depth, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_2D_ARRAY, a_flags, a_width, a_height, a_depth)
        {
            
        }
        
        Texture2DArray(const Texture2DArray<T, Channels> &texture) :
            Texture<T, Channels>(texture)
        {
            
        }
        
        Texture2DArray(const tiny::img::Image &image, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_2D_ARRAY, a_flags, image.width, image.height, 1)
        {
            for (size_t y = 0; y < image.height; ++y)
            {
                for (size_t x = 0; x < image.width; ++x)
                {
                    for (size_t i = 0; i < Channels; ++i) this->hostData[Channels*(x + image.width*y) + i] = image.data[4*(x + image.width*y) + i];
                }
            }
            
            this->sendToDevice();
        }
        
        template<typename ImageIterator>
        Texture2DArray(ImageIterator first, ImageIterator last, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_2D_ARRAY, a_flags, first->width, first->height, last - first)
        {
            size_t offset = 0;
            
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
                
                offset += this->width*this->height;
            }
            
            this->sendToDevice();
        }
        
        ~Texture2DArray()
        {
            
        }
};

typedef Texture2DArray<unsigned char, 3> RGBTexture2DArray;
typedef Texture2DArray<unsigned char, 4> RGBATexture2DArray;

}

}

