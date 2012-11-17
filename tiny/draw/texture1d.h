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
class Texture1D : public Texture<T, Channels>
{
    public:
        Texture1D(const size_t &width, const size_t &height) :
            Texture<T, Channels>(GL_TEXTURE_1D, width, height)
        {
            
        }
        
        Texture1D(const Texture1D<T, Channels> &texture)
            Texture<T, Channels>(texture)
        {
            
        }
        
        ~Texture1D()
        {
            
        }
        
        T & operator () (const size_t &a_x)
        {
            return hostData[a_x];
        }
        
        const T & operator () (const size_t &a_x) const
        {
            return hostData[a_x];
        }
};

}

}

