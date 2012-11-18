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

#include <tiny/draw/glcheck.h>
#include <tiny/draw/detail/formats.h>

namespace tiny
{

namespace draw
{

class TextureInterface
{
    public:
        TextureInterface(const GLenum &a_textureTarget, const GLint &a_textureFormat, const GLenum &a_textureChannels, const GLenum &a_textureDataType, const size_t &a_width, const size_t &a_height = 1, const size_t &a_depth = 1);
        TextureInterface(const TextureInterface &a_texture);
        virtual ~TextureInterface();
        
        GLuint getIndex() const;
        size_t getWidth() const;
        size_t getHeight() const;
        size_t getDepth() const;
        void bind(const int & = 0) const;
        void unbind(const int & = 0) const;
        
    protected:
        void createDeviceTexture();
        void destroyDeviceTexture();
        
        const GLenum textureTarget;
        const GLint textureFormat;
        const GLenum textureChannels;
        const GLenum textureDataType;
        const size_t width, height, depth;
        GLuint textureIndex;
};

template<typename T, size_t Channels>
class Texture : public TextureInterface
{
    public:
        Texture(const GLenum &a_textureTarget, const size_t &a_width, const size_t &a_height = 1, const size_t &a_depth = 1) :
            TextureInterface(a_textureTarget,
                             detail::getOpenGLTextureFormat<Channels, T>(),
                             detail::getOpenGLChannelType<Channels>(),
                             detail::getOpenGLDataType<T>(),
                             a_width,
                             a_height,
                             a_depth),
            hostData(a_width*a_height*a_depth*Channels)
        {
            if (hostData.empty())
                throw std::bad_alloc();
        }
        
        Texture(const Texture<T, Channels> &a_texture) :
            TextureInterface(a_texture),
            hostData(a_texture.hostData)
        {
            sendToDevice();
        }
        
        virtual ~Texture()
        {
            
        }
        
        void sendToDevice() const
        {
            if (hostData.empty()) return;
            
            GL_CHECK(glBindTexture(textureTarget, textureIndex));
            
                 if (textureTarget == GL_TEXTURE_1D) GL_CHECK(glTexSubImage1D(textureTarget, 0, 0, width, textureChannels, textureDataType, &hostData[0]));
            else if (textureTarget == GL_TEXTURE_2D) GL_CHECK(glTexSubImage2D(textureTarget, 0, 0, 0, width, height, textureChannels, textureDataType, &hostData[0]));
            else if (textureTarget == GL_TEXTURE_3D) GL_CHECK(glTexSubImage3D(textureTarget, 0, 0, 0, 0, width, height, depth, textureChannels, textureDataType, &hostData[0]));
            else throw std::exception();
            
            GL_CHECK(glBindTexture(textureTarget, 0));
        }
        
        bool empty() const
        {
            return hostData.empty();
        }
        
        size_t size() const
        {
            return hostData.size();
        }
        
        T & operator [] (const size_t &a_index)
        {
            return hostData[a_index];
        }
        
        const T & operator [] (const size_t &a_index) const
        {
            return hostData[a_index];
        }
        
        typename std::vector<T>::iterator begin()
        {
            return hostData.begin();
        }
        
        typename std::vector<T>::const_iterator begin() const
        {
            return hostData.begin();
        }
        
        typename std::vector<T>::iterator end()
        {
            return hostData.end();
        }
        
        typename std::vector<T>::const_iterator end() const
        {
            return hostData.end();
        }
        
    protected:
        std::vector<T> hostData;
};

}

}

