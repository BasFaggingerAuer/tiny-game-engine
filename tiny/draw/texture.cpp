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
#include <iostream>

#include <tiny/draw/texture.h>

using namespace tiny::draw;

TextureInterface::TextureInterface(const GLenum &a_textureTarget,
                                   const GLint &a_textureFormat,
                                   const GLenum &a_textureChannels,
                                   const GLenum &a_textureDataType,
                                   const size_t &a_nrChannels,
                                   const unsigned int &a_flags,
                                   const size_t &a_width,
                                   const size_t &a_height,
                                   const size_t &a_depth) :
    textureTarget(a_textureTarget),
    textureFormat(a_textureFormat),
    textureChannels(a_textureChannels),
    textureDataType(a_textureDataType),
    flags(a_flags),
    width(a_width),
    height(a_height),
    depth(a_depth),
    nrChannels(a_nrChannels),
    textureIndex(0)
{
    createDeviceTexture();
}

TextureInterface::TextureInterface(const TextureInterface &a_texture) :
    textureTarget(a_texture.textureTarget),
    textureFormat(a_texture.textureFormat),
    textureChannels(a_texture.textureChannels),
    textureDataType(a_texture.textureDataType),
    flags(a_texture.flags),
    width(a_texture.width),
    height(a_texture.height),
    depth(a_texture.depth),
    nrChannels(a_texture.nrChannels),
    textureIndex(0)
{
    createDeviceTexture();
}

TextureInterface::~TextureInterface()
{
    destroyDeviceTexture();
}

GLuint TextureInterface::getIndex() const
{
    return textureIndex;
}

size_t TextureInterface::getWidth() const
{
    return width;
}

size_t TextureInterface::getHeight() const
{
    return height;
}

size_t TextureInterface::getDepth() const
{
    return depth;
}

size_t TextureInterface::getChannels() const
{
    return nrChannels;
}

void TextureInterface::bind(const int &bindTarget) const
{
    //std::cerr << "Binding texture " << textureIndex << " to " << bindTarget << " on target " << textureTarget << " (GL_TEXTURE_2D = " << GL_TEXTURE_2D << ")." << std::endl;
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + bindTarget));
    GL_CHECK(glBindTexture(textureTarget, textureIndex));
}

void TextureInterface::unbind(const int &bindTarget) const
{
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + bindTarget));
    GL_CHECK(glBindTexture(textureTarget, 0));
}

void TextureInterface::createDeviceTexture()
{
    GL_CHECK(glGenTextures(1, &textureIndex));
    
    if (textureIndex == 0)
        throw std::bad_alloc();
    
    GL_CHECK(glBindTexture(textureTarget, textureIndex));
         
    if (textureTarget == GL_TEXTURE_1D)
    {
        GL_CHECK(glTexImage1D(textureTarget, 0, textureFormat, width, 0, textureChannels, textureDataType, 0));
    }
    else if (textureTarget == GL_TEXTURE_2D)
    {
        GL_CHECK(glTexImage2D(textureTarget, 0, textureFormat, width, height, 0, textureChannels, textureDataType, 0));
    }
    else if (textureTarget == GL_TEXTURE_3D)
    {
        GL_CHECK(glTexImage3D(textureTarget, 0, textureFormat, width, height, depth, 0, textureChannels, textureDataType, 0));
    }
    else if (textureTarget == GL_TEXTURE_2D_ARRAY)
    {
        GL_CHECK(glTexImage3D(textureTarget, 0, textureFormat, width, height, depth, 0, textureChannels, textureDataType, 0));
    }
    else if (textureTarget == GL_TEXTURE_CUBE_MAP_ARRAY)
    {
        GL_CHECK(glTexImage3D(textureTarget, 0, textureFormat, width, height, depth, 0, textureChannels, textureDataType, 0));
    }
    else if (textureTarget == GL_TEXTURE_BUFFER)
    {
        //No action is required.
    }
    else
    {
        std::cerr << "Invalid texture target!" << std::endl;
        throw std::exception();
    }
    
    if (textureTarget != GL_TEXTURE_BUFFER)
    {
        GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, (flags & tf::repeat) != 0 ? GL_REPEAT : GL_CLAMP));
        GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, (flags & tf::repeat) != 0 ? GL_REPEAT : GL_CLAMP));
        
        if ((flags & tf::mipmap) != 0)
        {
            GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, (flags & tf::filter) != 0 ? GL_LINEAR : GL_NEAREST));
            GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, (flags & tf::filter) != 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR));
            GL_CHECK(glGenerateMipmap(textureTarget));
        }
        else
        {
            GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, (flags & tf::filter) != 0 ? GL_LINEAR : GL_NEAREST));
            GL_CHECK(glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, (flags & tf::filter) != 0 ? GL_LINEAR : GL_NEAREST));
        }
    }
    
    GL_CHECK(glBindTexture(textureTarget, 0));
}

void TextureInterface::destroyDeviceTexture()
{
    if (textureIndex != 0) GL_CHECK(glDeleteTextures(1, &textureIndex));
    
    textureIndex = 0;
}

