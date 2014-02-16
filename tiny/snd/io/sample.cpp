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

#include <SDL.h>
#include <SDL_mixer.h>

#include <tiny/snd/io/sample.h>

using namespace tiny::snd;

Sample *tiny::snd::io::readSample(const std::string &fileName)
{
    //Read sample from disk.
    Sample *sample = new Sample();
    
    sample->chunk = Mix_LoadWAV(fileName.c_str());
    
    if (!sample->chunk)
    {
        std::cerr << "Unable to read '" << fileName << "': " << Mix_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    std::cerr << "Read a sample from '" << fileName << "'." << std::endl;
    
    return sample;
}

