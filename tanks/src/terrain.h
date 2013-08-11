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

#include <tinyxml.h>

#include <tiny/draw/terrain.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/texture2darray.h>

namespace tanks
{

class TanksTerrain
{
    public:
        TanksTerrain(const std::string &, TiXmlElement *);
        ~TanksTerrain();
        
        void setOffset(const tiny::vec2 &);
        void calculateAttributes(const tiny::draw::FloatTexture2D &, tiny::draw::RGBATexture2D &, const float &) const;
        
        float getHeight(const tiny::vec2 &) const;
        tiny::vec4 getAttributes(const tiny::vec2 &) const;
        
        tiny::draw::Terrain *terrain;
        tiny::vec2 terrainScale;
        tiny::ivec2 terrainFarScale;
        tiny::vec2 terrainDetailScale;
        tiny::vec2 terrainFarOffset;
        std::string terrainAttributeShader;
        tiny::draw::FloatTexture2D *terrainHeightTexture;
        tiny::draw::FloatTexture2D *terrainFarHeightTexture;
        tiny::draw::RGBTexture2D *terrainNormalTexture;
        tiny::draw::RGBTexture2D *terrainFarNormalTexture;
        tiny::draw::RGBTexture2D *terrainTangentTexture;
        tiny::draw::RGBTexture2D *terrainFarTangentTexture;
        tiny::draw::RGBATexture2D *terrainAttributeTexture;
        tiny::draw::RGBATexture2D *terrainFarAttributeTexture;
        
        tiny::draw::RGBTexture2DArray *biomeDiffuseTextures;
        tiny::draw::RGBTexture2DArray *biomeNormalTextures;
        
    private:
        //A simple bilinear texture sampler, which converts world coordinates to the corresponding texture coordinates on the zoomed-in terrain.
        template<typename TextureType>
        static tiny::vec4 sampleTextureBilinear(const TextureType &texture, const tiny::vec2 &scale, const tiny::vec2 &a_pos)
        {
            //Sample texture at the four points surrounding pos.
            const tiny::vec2 pos = tiny::vec2(a_pos.x/scale.x + 0.5f*static_cast<float>(texture.getWidth()), a_pos.y/scale.y + 0.5f*static_cast<float>(texture.getHeight()));
            const tiny::ivec2 intPos = tiny::ivec2(floor(pos.x), floor(pos.y));
            const tiny::vec4 h00 = texture(intPos.x + 0, intPos.y + 0);
            const tiny::vec4 h01 = texture(intPos.x + 0, intPos.y + 1);
            const tiny::vec4 h10 = texture(intPos.x + 1, intPos.y + 0);
            const tiny::vec4 h11 = texture(intPos.x + 1, intPos.y + 1);
            const tiny::vec2 delta = tiny::vec2(pos.x - floor(pos.x), pos.y - floor(pos.y));
            
            //Interpolate between these four points.
            return delta.y*(delta.x*h11 + (1.0f - delta.x)*h01) + (1.0f - delta.y)*(delta.x*h10 + (1.0f - delta.x)*h00);
        }
};

} //namespace tanks

