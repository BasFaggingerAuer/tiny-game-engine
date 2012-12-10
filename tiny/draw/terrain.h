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

#include <tiny/math/vec.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/detail/terrain.h>

namespace tiny
{

namespace draw
{

class Terrain : public Renderable
{
    public:
        Terrain();
        ~Terrain();
        
        template <typename TextureType>
        void setHeightTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "heightTexture");
        }
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        int minLevel;
        int maxLevel;
        vec3 scale;
        ivec2 bitShifts;
        ivec2 heightMapSize;
        std::vector<ivec2> blockTranslations;
        
        detail::TerrainBlock smallBlock;
        detail::TerrainBlock largeBlock;
        detail::TerrainBlock crossBlockX;
        detail::TerrainBlock crossBlockY;
        detail::TerrainBlock ellBlockX;
        detail::TerrainBlock ellBlockY;
};

}

}
