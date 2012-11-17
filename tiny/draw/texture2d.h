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

#include <tiny/draw/texture.h>

namespace tiny
{

namespace draw
{

template<typename T, size_t Channels>
class Texture2D : public Texture<T, Channels>
{
    public:
        Texture2D(const size_t &width, const size_t &height) :
            Texture<T, Channels>(GL_TEXTURE_2D, width, height)
        {
            
        }
        
        Texture2D(const Texture2D<T, Channels> &texture) :
            Texture<T, Channels>(texture)
        {
            
        }
        
        ~Texture2D()
        {
            
        }
        
        T & operator () (const size_t &a_x, const size_t &a_y)
        {
            return this->hostData[a_x + this->width*a_y];
        }
        
        const T & operator () (const size_t &a_x, const size_t &a_y) const
        {
            return this->hostData[a_x + this->width*a_y];
        }
};

}

}

