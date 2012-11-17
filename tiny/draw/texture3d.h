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

#include <texture.h>

namespace tiny
{

namespace draw
{

template<typename T, size_t Channels>
class Texture3D : public Texture<T, Channels>
{
    public:
        Texture3D(const size_t &width, const size_t &height) :
            Texture<T, Channels>(GL_TEXTURE_3D, width, height)
        {
            
        }
        
        Texture3D(const Texture3D<T, Channels> &texture)
            Texture<T, Channels>(texture)
        {
            
        }
        
        ~Texture3D()
        {
            
        }
        
        T & operator () (const size_t &a_x, const size_t &a_y, const size_t &a_z)
        {
            return hostData[a_x + width*a_y + width*height*a_z];
        }
        
        const T & operator () (const size_t &a_x, const size_t &a_y, const size_t &a_z) const
        {
            return hostData[a_x + width*a_y + width*height*a_z];
        }
};

}

}

