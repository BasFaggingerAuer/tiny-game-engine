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
#include <tiny/draw/buffer.h>

namespace tiny
{

namespace draw
{

template<typename T, size_t Channels>
class TextureBuffer : public Texture<T, Channels>
{
    public:
        TextureBuffer() :
            Texture<T, Channels>(GL_TEXTURE_BUFFER, 1, 1, 1)
        {
            unbindBuffer();
        }
        
        ~TextureBuffer()
        {
            
        }
        
        void bindBuffer(const BufferInterface &buffer) const
        {
            this->bind();
            GL_CHECK(glTexBuffer(GL_TEXTURE_BUFFER, this->textureFormat, buffer.getIndex()));
            this->unbind();
        }
        
        void unbindBuffer() const
        {
            this->bind();
            GL_CHECK(glTexBuffer(GL_TEXTURE_BUFFER, this->textureFormat, 0));
            this->unbind();
        }
};

typedef TextureBuffer<float, 4> Vec4TextureBuffer;

}

}

