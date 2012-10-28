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
#include <vector>

#include <cassert>

#include <GL/gl.h>

namespace tiny
{

namespace draw
{

namespace detail
{

template<>
GLenum getOpenGLDataType<float> {return GL_FLOAT;}
template<>
GLenum getOpenGLDataType<unsigned char> {return GL_UNSIGNED_BYTE;}
template<>
GLenum getOpenGLDataType<int> {return GL_INT;}

template<>
GLenum getOpenGLChannelType<1> {return GL_RED;}
template<>
GLenum getOpenGLChannelType<2> {return GL_RG;}
template<>
GLenum getOpenGLChannelType<3> {return GL_RGB;}
template<>
GLenum getOpenGLChannelType<4> {return GL_RGBA;}

template<>
GLint getOpenGLTextureFormat<1, unsigned char> {return GL_R8;}
template<>
GLint getOpenGLTextureFormat<2, unsigned char> {return GL_RG8;}
template<>
GLint getOpenGLTextureFormat<3, unsigned char> {return GL_RGB8;}
template<>
GLint getOpenGLTextureFormat<4, unsigned char> {return GL_RGBA8;}

template<>
GLint getOpenGLTextureFormat<1, float> {return GL_R32F;}
template<>
GLint getOpenGLTextureFormat<2, float> {return GL_RG32F;}
template<>
GLint getOpenGLTextureFormat<3, float> {return GL_RGB32F;}
template<>
GLint getOpenGLTextureFormat<4, float> {return GL_RGBA32F;}

}

}

}

