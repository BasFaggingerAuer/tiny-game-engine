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
#include <cassert>

#include <AL/al.h>
#include <AL/alc.h>

#include <tiny/snd/alcheck.h>
#include <tiny/snd/worldsounderer.h>

using namespace tiny;
using namespace tiny::snd;

WorldSounderer::WorldSounderer()
{
    
}

WorldSounderer::~WorldSounderer()
{

}

void WorldSounderer::setCamera(const vec3 &position, const vec4 &orientation, const vec3 &velocity)
{
    const mat4 m = mat4(orientation);
    const vec3 at = m*vec3(0.0f, 0.0f,-1.0f);
    const vec3 up = m*vec3(0.0f, 1.0f, 0.0f);
    const ALfloat ori[] = {at.x, at.y, at.z, up.x, up.y, up.z};
    
    AL_CHECK(alListener3f(AL_POSITION, position.x, position.y, position.z));
    AL_CHECK(alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z));
    AL_CHECK(alListenerfv(AL_ORIENTATION, ori));
}

