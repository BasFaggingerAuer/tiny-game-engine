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
class Texture3D : public Texture<T, Channels>
{
    public:
        Texture3D(const size_t &a_width, const size_t &a_height, const size_t &a_depth, const unsigned int &a_flags = tf::repeat | tf::filter | tf::mipmap) :
            Texture<T, Channels>(GL_TEXTURE_3D, a_flags, a_width, a_height, a_depth)
        {
            
        }
        
        Texture3D(const Texture3D<T, Channels> &texture) :
            Texture<T, Channels>(texture)
        {
            
        }
        
        ~Texture3D()
        {
            
        }
        
        vec4 operator () (const size_t &, const size_t &, const size_t &) const;
};

//Functions to sample different types of textures.
template <>
inline vec4 Texture3D<unsigned char, 1>::operator () (const size_t &a_x, const size_t &a_y, const size_t &a_z) const
{
    if (a_x >= width || a_y >= height || a_z >= depth)
    {
        return vec4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    return vec4(static_cast<float>(this->hostData[1*(a_x + this->width*a_y) + 0])/255.0f,
                0.0f,
                0.0f,
                1.0f);
}

typedef Texture3D<float, 1> FloatTexture3D;
typedef Texture3D<float, 3> Vec3Texture3D;
typedef Texture3D<float, 4> Vec4Texture3D;
typedef Texture3D<unsigned char, 1> RTexture3D;
typedef Texture3D<unsigned char, 3> RGBTexture3D;
typedef Texture3D<unsigned char, 4> RGBATexture3D;

}

}

