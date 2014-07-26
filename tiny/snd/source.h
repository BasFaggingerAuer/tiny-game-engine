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

#include <iostream>
#include <exception>
#include <vector>

#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>

#include <tiny/math/vec.h>

#include <tiny/snd/alcheck.h>
#include <tiny/snd/buffer.h>

namespace tiny
{

namespace snd
{

class Source
{
    public:
        Source(const vec3 &a_position = vec3(0.0f, 0.0f, 0.0f));
        ~Source();
        
        void setPosition(const vec3 &a_position, const vec3 &a_velocity = vec3(0.0f, 0.0f, 0.0f));
        void setPitch(const float &a_pitch);
        void setGain(const float &a_gain);
        
        template<typename T, size_t Channels>
        void playBuffer(const Buffer<T, Channels> &a_buffer, const bool &a_looping, const float &a_offset = 0.0f)
        {
            //Play the given buffer with this source.
            AL_CHECK(alSourcei(sourceIndex, AL_LOOPING, a_looping ? AL_TRUE : AL_FALSE));
            AL_CHECK(alSourcef(sourceIndex, AL_SEC_OFFSET, a_offset));
            AL_CHECK(alSourcei(sourceIndex, AL_BUFFER, a_buffer.getIndex()));
            AL_CHECK(alSourcePlay(sourceIndex));
        }
        
        void stopPlaying();
        
    private:
        ALuint sourceIndex;
};

}

}

