/*
Copyright 2015, Matthijs van Dorp.

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
#include <cassert>
#include <string>
#include <vector>

namespace tiny
{

namespace draw
{
    class Colour
    {
        private:
            unsigned char r; /**< Red shade from 0 to 255. */
            unsigned char g; /**< Green shade from 0 to 255. */
            unsigned char b; /**< Blue shade from 0 to 255. */
            unsigned char a; /**< Alpha shade from 0 to 255. */
        public:
            Colour(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255) :
                r(_r), g(_g), b(_b), a(_a) {}

            vec4 toVector(void) const { return vec4( (1.0f/255.0f)*r, (1.0f/255.0f)*g, (1.0f/255.0f)*b, (1.0f/255.0f)*a); }
    };
} // end namespace tiny

} // end namespace draw
