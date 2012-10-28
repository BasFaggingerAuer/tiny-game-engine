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

#include <exception>
#include <cassert>

#include <GL/gl.h>

#include <draw/texture2d.h>

namespace tiny
{

namespace draw
{

/*! \p RenderTarget : maps rendering to the screen and/or textures.
 */
class RenderTarget
{
    public:
        RenderTarget();
        ~RenderTarget();
        
        void bind() const;
        void unbind() const;
        
        template<typename T, size_t Channels>
        void attachColorTarget(const Texture2D<T, Channels> &texture, const size_t &index)
        {
            glBindFrameBuffer(GL_FRAMEBUFFER, frameBufferIndex);
            glFrameBufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, texture.getIndex(), 0);
            glBindFrameBuffer(GL_FRAMEBUFFER, 0);
        }
        
    private:
        RenderTarget(const RenderTarget &);
        
        GLuint frameBufferIndex;
};

}

}

