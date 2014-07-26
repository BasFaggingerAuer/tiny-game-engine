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
#include <vector>

#include <SDL.h>
#include <SDL_endian.h>

#include <vorbis/vorbisfile.h>

#include <tiny/smp/io/sample.h>

using namespace tiny::smp;

Sample tiny::smp::io::readSample(const std::string &fileName)
{
    //Read sample from disk.
    OggVorbis_File file;
    
    const int fopenStatus = ov_fopen(fileName.c_str(), &file);
    
    if (fopenStatus != 0)
    {
        std::cerr << "Unable to open '" << fileName << "' for reading: " << fopenStatus << "!" << std::endl;
        throw std::exception();
    }
    
    //Retrieve file information.
    const vorbis_info *fileInfo = ov_info(&file, -1);
    Sample sample(fileInfo->rate, fileInfo->channels);
    
    //Read file in chunks.
    const size_t bufferSize = 65536;
    std::vector<short> buffer(bufferSize/sizeof(short), 0);
    long nrBytesRead = 0;
    int bitStream = 0;
    
    do
    {
        nrBytesRead = ov_read(&file, reinterpret_cast<char *>(&buffer[0]), bufferSize, (SDL_BYTEORDER == SDL_BIG_ENDIAN ? 1 : 0), sizeof(short), 1, &bitStream);
        sample.data.insert(sample.data.end(), buffer.begin(), buffer.begin() + (nrBytesRead/sizeof(short)));
    } while(nrBytesRead > 0);
    
    ov_clear(&file);
    
    std::cerr << "Read a " << sample.channels << " channel file at " << sample.frequency << "Hz from '" << fileName << "'." << std::endl;
    return sample;
}

