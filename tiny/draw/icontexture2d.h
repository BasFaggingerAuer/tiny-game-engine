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

#include <vector>
#include <list>
#include <map>

#include <tiny/math/vec.h>
#include <tiny/draw/texture2d.h>
#include <tiny/img/image.h>

namespace tiny
{

namespace draw
{

class IconTexture2D : public Texture2D<unsigned char, 4>
{
    public:
        IconTexture2D(const size_t &, const size_t &);
        ~IconTexture2D();
        
        void clearIcons();
		vec4 packIcon(const img::Image &);
		void packIcons(const std::vector<img::Image> &);
		vec4 getIcon(const int &) const;
        vec2 getMaxIconDimensions() const;
        
	private:
		vec4 addSingleIcon(const img::Image &);
		
		std::map<ivec2, bool> occupied;
		std::list<int> xBounds, yBounds;
		std::vector<ivec4> subImages;
		ivec2 maxSubImageDimensions;
};

}

}

