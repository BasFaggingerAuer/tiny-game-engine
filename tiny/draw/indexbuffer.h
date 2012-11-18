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

#include <tiny/draw/buffer.h>

namespace tiny
{

namespace draw
{

/*! \p IndexBuffer : data buffer on an OpenGL device designed for holding vertex data.
 * 
 * \tparam T type of object stored in this buffer
 */
template<typename T>
class IndexBuffer : public Buffer<T>
{
    public:
        IndexBuffer(const size_t &a_size) :
            Buffer<T>(a_size, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
        {

        }

        IndexBuffer(const IndexBuffer &a_buffer) :
            Buffer<T>(a_buffer)
        {

        }
        
        template <typename Iterator>
        IndexBuffer(Iterator first, Iterator last) :
            Buffer<T>(first, last, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW)
        {

        }
        
        virtual ~IndexBuffer()
        {

        }
};

}

}

