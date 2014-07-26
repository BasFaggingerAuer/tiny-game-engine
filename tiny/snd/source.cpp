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
#include <tiny/snd/source.h>

using namespace tiny::snd;

Source::Source(const vec3 &a_position)
{
    AL_CHECK(alGenSources(1, &sourceIndex));
    AL_CHECK(alSource3f(sourceIndex, AL_POSITION, a_position.x, a_position.y, a_position.z));
}

Source::~Source()
{
    if (sourceIndex != 0)
    {
        AL_CHECK(alDeleteSources(1, &sourceIndex));
    }
}

void Source::setPosition(const vec3 &a_position, const vec3 &a_velocity)
{
    AL_CHECK(alSource3f(sourceIndex, AL_POSITION, a_position.x, a_position.y, a_position.z));
    AL_CHECK(alSource3f(sourceIndex, AL_POSITION, a_velocity.x, a_velocity.y, a_velocity.z));
}

void Source::setPitch(const float &a_pitch)
{
    AL_CHECK(alSourcef(sourceIndex, AL_PITCH, a_pitch));
}

void Source::setGain(const float &a_gain)
{
    AL_CHECK(alSourcef(sourceIndex, AL_GAIN, a_gain));
}

void Source::stopPlaying()
{
    AL_CHECK(alSourceStop(sourceIndex));
    AL_CHECK(alSourcei(sourceIndex, AL_BUFFER, 0));
}

