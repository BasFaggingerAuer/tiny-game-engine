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
#include <tiny/draw/texture.h>

using namespace tiny::draw;

TextureInterface::TextureInterface(const GLenum &a_textureTarget,
                                   const GLint &a_textureFormat,
                                   const GLenum &a_textureChannels,
                                   const GLenum &a_textureDataType,
                                   const size_t &a_width,
                                   const size_t &a_height,
                                   const size_t &a_depth) :
    textureTarget(a_textureTarget),
    textureFormat(a_textureFormat),
    textureChannels(a_textureChannels),
    textureDataType(a_textureDataType),
    width(a_width),
    height(a_height),
    depth(a_depth),
    textureIndex(0)
{
    createDeviceTexture();
}

TextureInterface::TextureInterface(const TextureInterface &a_texture) :
    textureTarget(a_texture.textureTarget),
    textureFormat(a_texture.textureFormat),
    textureChannels(a_texture.textureChannels),
    textureDataType(a_texture.textureDataType),
    width(a_texture.width),
    height(a_texture.height),
    depth(a_texture.depth),
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

void TextureInterface::bind(const int &bindTarget) const
{
    glActiveTexture(GL_TEXTURE0 + bindTarget);
    glBindTexture(textureTarget, textureIndex);
}

void TextureInterface::unbind(const int &bindTarget) const
{
    glActiveTexture(GL_TEXTURE0 + bindTarget);
    glBindTexture(textureTarget, 0);
}

void TextureInterface::createDeviceTexture()
{
    glGenTextures(1, &textureIndex);
    
    if (textureIndex == 0)
        throw std::bad_alloc();
    
    glBindTexture(textureTarget, textureIndex);
    
         if (textureTarget == GL_TEXTURE_1D) glTexImage1D(textureTarget, 0, textureFormat, width, 0, textureChannels, textureDataType, 0);
    else if (textureTarget == GL_TEXTURE_2D) glTexImage2D(textureTarget, 0, textureFormat, width, height, 0, textureChannels, textureDataType, 0);
    else if (textureTarget == GL_TEXTURE_3D) glTexImage3D(textureTarget, 0, textureFormat, width, height, depth, 0, textureChannels, textureDataType, 0);
    else throw std::exception();
    
    glBindTexture(textureTarget, 0);
}

void TextureInterface::destroyDeviceTexture()
{
    if (textureIndex != 0) glDeleteTextures(1, &textureIndex);
    
    textureIndex = 0;
}

