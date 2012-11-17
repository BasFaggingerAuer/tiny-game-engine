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

class BufferInterface
{
    public:
        BufferInterface(const size_t &a_sizeInBytes, const GLenum &a_target, const GLenum &a_usage);
        BufferInterface(const BufferInterface &a_buffer);
        virtual ~BufferInterface();
        
        GLuint getIndex() const;
        void bind() const;
        void unbind() const;
        
    protected:
        void createDeviceBuffer();
        void destroyDeviceBuffer();
        void resizeDeviceBuffer(const size_t &a_sizeInBytes);
        
        size_t sizeInBytes;
        const GLenum target;
        const GLenum usage;
        GLuint bufferIndex;
};

/*! \p Buffer : data buffer on an OpenGL device.
 * 
 * \tparam T type of object stored in this buffer
 */
template<typename T>
class Buffer : public BufferInterface
{
    public:
        Buffer(const size_t &a_size, const GLenum &a_target, const GLenum &a_usage) :
            BufferInterface(a_size*sizeof(T), a_target, a_usage),
            hostData(a_size)
        {
            if (hostData.empty())
                throw std::bad_alloc();
        }
        
        Buffer(const Buffer<T> &a_buffer) :
            BufferInterface(a_buffer),
            hostData(a_buffer.hostData)
        {
            sendToDevice();
        }
        
        virtual ~Buffer()
        {
            
        }
        
        void sendToDevice() const
        {
            assert(hostData.size()*sizeof(T) == sizeInBytes);
            
            if (hostData.empty()) return;
            
            glBindBuffer(target, bufferIndex);
            glBufferSubData(target, 0, sizeInBytes, &hostData[0]);
            glBindBuffer(target, 0);
        }
        
        bool empty() const
        {
            return (hostData.empty() || sizeInBytes == 0);
        }
        
        size_t size() const
        {
            return hostData.size();
        }
        
        void resize(const size_t &a_size)
        {
            resizeDeviceBuffer(a_size*sizeof(T));
            hostData.resize(a_size);
        }
        
        void assign(const size_t &a_size, const T &a_copy)
        {
            resizeDeviceBuffer(a_size*sizeof(T));
            hostData.assign(a_size, a_copy);
            sendToDevice();
        }
        
        template <class InputIterator>
        void assign(InputIterator first, InputIterator last)
        {
            resizeDeviceBuffer((last - first)*sizeof(T));
            hostData.assign(first, last);
            sendToDevice();
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

