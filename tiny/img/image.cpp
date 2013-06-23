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

#include <tiny/img/image.h>

using namespace tiny::img;

Image::Image() :
    width(0),
    height(0),
    data()
{

}

Image::Image(const size_t &a_width, const size_t &a_height) :
    width(a_width),
    height(a_height),
    data(width*height*4, 0)
{

}

Image::~Image()
{

}

Image Image::createTestImage(const size_t &size)
{
    Image test(size, size);
    unsigned char *data = &test.data[0];
    
    if (size <= 0)
    {
        std::cerr << "Unable to create empty test image." << std::endl;
        throw std::exception();
    }
    
    //Create a simple test image.
    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            const unsigned char colour = (((2*i)/size)^((2*j)/size))*255;
            
            *data++ = colour;
            *data++ = colour;
            *data++ = colour;
            *data++ = 255;
        }
    }
    
    return test;
}

