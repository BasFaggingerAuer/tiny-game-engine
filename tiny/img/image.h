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

#include <string>
#include <vector>

namespace tiny
{

namespace img
{

class Image
{
    public:
        Image();
        Image(const size_t &, const size_t &);
        ~Image();
        
        static Image createSolidImage(const size_t & = 4, const unsigned char & = 255, const unsigned char & = 255, const unsigned char & = 255, const unsigned char & = 255);
        static Image createTestImage(const size_t & = 64);
        static Image createUpNormalImage(const size_t & = 4);

        Image flipUpDown() const;
        
        size_t width, height;
        std::vector<unsigned char> data;
};

}

}

