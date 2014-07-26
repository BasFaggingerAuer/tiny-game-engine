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

namespace smp
{

class Sample
{
    public:
        Sample();
        Sample(const size_t &, const size_t &);
        ~Sample();
        
        static Sample createTone(const float & = 440.0, const float & = 44100.0f);
        
        size_t frequency;
        size_t channels;
        std::vector<short> data;
};

}

}

