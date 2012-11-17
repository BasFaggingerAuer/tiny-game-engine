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

#include <GL/glew.h>
#include <GL/gl.h>

namespace tiny
{

namespace draw
{

namespace detail
{

template<typename T>
inline GLenum getOpenGLDataType() {return GL_UNSIGNED_BYTE;}

template<>
inline GLenum getOpenGLDataType<float>() {return GL_FLOAT;}
template<>
inline GLenum getOpenGLDataType<unsigned char>() {return GL_UNSIGNED_BYTE;}
template<>
inline GLenum getOpenGLDataType<int>() {return GL_INT;}
template<>
inline GLenum getOpenGLDataType<unsigned int>() {return GL_UNSIGNED_INT;}

template<size_t T>
inline GLenum getOpenGLChannelType() {return GL_RED;}

template<>
inline GLenum getOpenGLChannelType<1>() {return GL_RED;}
template<>
inline GLenum getOpenGLChannelType<2>() {return GL_RG;}
template<>
inline GLenum getOpenGLChannelType<3>() {return GL_RGB;}
template<>
inline GLenum getOpenGLChannelType<4>() {return GL_RGBA;}

template<size_t T, typename S>
inline GLint getOpenGLTextureFormat() {return GL_R8;}

template<>
inline GLint getOpenGLTextureFormat<1, unsigned char>() {return GL_R8;}
template<>
inline GLint getOpenGLTextureFormat<2, unsigned char>() {return GL_RG8;}
template<>
inline GLint getOpenGLTextureFormat<3, unsigned char>() {return GL_RGB8;}
template<>
inline GLint getOpenGLTextureFormat<4, unsigned char>() {return GL_RGBA8;}

template<>
inline GLint getOpenGLTextureFormat<1, float>() {return GL_R32F;}
template<>
inline GLint getOpenGLTextureFormat<2, float>() {return GL_RG32F;}
template<>
inline GLint getOpenGLTextureFormat<3, float>() {return GL_RGB32F;}
template<>
inline GLint getOpenGLTextureFormat<4, float>() {return GL_RGBA32F;}

}

}

}

