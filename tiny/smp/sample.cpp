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
#define _USE_MATH_DEFINES
#include <cmath>

#include <tiny/smp/sample.h>

using namespace tiny::smp;

Sample::Sample() :
    frequency(0),
    channels(0),
    data()
{

}

Sample::Sample(const size_t &a_frequency, const size_t &a_channels) :
    frequency(a_frequency),
    channels(a_channels),
    data()
{

}

Sample::~Sample()
{

}

Sample Sample::createTone(const float &toneFrequency, const float &sampleFrequency)
{
    //Create a sine wave of the desired frequency.
    Sample sample(static_cast<size_t>(sampleFrequency), 1);
    
    //Sample one entire period.
    const size_t nrSamples = static_cast<size_t>(ceil(sampleFrequency/toneFrequency));
    
    sample.data.assign(nrSamples, 0);
    
    for (size_t i = 0; i < nrSamples; ++i)
    {
        sample.data[i] = static_cast<short>(0.5f*32767.5f*(1.0f + sin(2.0f*M_PI*static_cast<float>(i)/static_cast<float>(nrSamples))));
    }
    
    return sample;
}

Sample Sample::createBlockTone(const float &toneFrequency, const float &sampleFrequency)
{
    //Create a sine wave of the desired frequency.
    Sample sample(static_cast<size_t>(sampleFrequency), 1);
    
    //Sample one entire period.
    const size_t nrSamples = static_cast<size_t>(ceil(sampleFrequency/toneFrequency));
    
    sample.data.assign(nrSamples, 0);
    
    for (size_t i = 0; i < nrSamples/2; ++i) sample.data[i] = 32767;
    for (size_t i = nrSamples/2; i < nrSamples; ++i) sample.data[i] = -32767;
    
    return sample;
}
