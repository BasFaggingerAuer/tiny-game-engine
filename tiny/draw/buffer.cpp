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
#include <tiny/draw/buffer.h>

using namespace tiny::draw;

BufferInterface::BufferInterface(const size_t &a_sizeInBytes, const GLenum &a_target, const GLenum &a_usage) :
    sizeInBytes(0),
    target(a_target),
    usage(a_usage),
    bufferIndex(0)
{
    resizeDeviceBuffer(a_sizeInBytes);
}

BufferInterface::BufferInterface(const BufferInterface &a_buffer) :
    sizeInBytes(0),
    target(a_buffer.target),
    usage(a_buffer.usage),
    bufferIndex(0)
{
    resizeDeviceBuffer(a_buffer.sizeInBytes);
}

BufferInterface::~BufferInterface()
{
    destroyDeviceBuffer();
}

GLuint BufferInterface::getIndex() const
{
    return bufferIndex;
}

void BufferInterface::bind() const
{
    GL_CHECK(glBindBuffer(target, bufferIndex));
}

void BufferInterface::unbind() const
{
    GL_CHECK(glBindBuffer(target, 0));
}

void BufferInterface::createDeviceBuffer()
{
    GL_CHECK(glGenBuffers(1, &bufferIndex));
    
    if (bufferIndex == 0)
        throw std::bad_alloc();
}

void BufferInterface::destroyDeviceBuffer()
{
    //Frees all data bound to this class on the device.
    if (bufferIndex != 0) GL_CHECK(glDeleteBuffers(1, &bufferIndex));
    
    sizeInBytes = 0;
    bufferIndex = 0;
}

void BufferInterface::resizeDeviceBuffer(const size_t &a_sizeInBytes)
{
    //Do not perform needless re-allocation.
    if (sizeInBytes == a_sizeInBytes) return;
    
    //Create new device array.
    sizeInBytes = a_sizeInBytes;
    
    if (sizeInBytes == 0)
    {
        destroyDeviceBuffer();
        return;
    }
    
    //Allocate new buffer if necessary.
    if (bufferIndex == 0) createDeviceBuffer();
    
    //Resize buffer.
    GL_CHECK(glBindBuffer(target, bufferIndex));
    GL_CHECK(glBufferData(target, sizeInBytes, 0, usage));
    GL_CHECK(glBindBuffer(target, 0));
}

