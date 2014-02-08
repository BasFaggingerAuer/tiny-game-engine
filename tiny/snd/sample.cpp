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
#include <exception>

#include <SDL_mixer.h>

#include <tiny/snd/sample.h>

using namespace tiny::snd;

Sample::Sample() :
    chunk(0)
{
    
}

Sample::~Sample()
{
    
}

void tiny::snd::playSample(const Sample &sample, const int &channel, const int &nrLoops)
{
    //Play a given sample at a desired channel.
    if (!sample.chunk) return;
    
    if (Mix_PlayChannel(channel, sample.chunk, nrLoops) == -1)
    {
        std::cerr << "Unable to play sample: " << Mix_GetError() << "!";
    }
}

