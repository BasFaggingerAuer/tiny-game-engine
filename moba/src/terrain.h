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

#include <tinyxml.h>

#include <tiny/img/image.h>

#include <tiny/draw/terrain.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/texture2darray.h>

#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/iconhorde.h>

namespace moba
{

class GameTerrain
{
    public:
        GameTerrain(const std::string &, TiXmlElement *);
        ~GameTerrain();
        
        void setOffset(const tiny::vec2 &);
        int createAttributeMapSamples(const int &, const int &, std::vector<tiny::draw::StaticMeshInstance> &, const tiny::vec2 &, std::vector<tiny::draw::WorldIconInstance> &, std::vector<tiny::vec3> &) const;
        tiny::vec3 getWorldPosition(const float &, const float &) const;
        
        float getHeight(const tiny::vec2 &) const;
        float getAttribute(const tiny::vec2 &) const;
        
        tiny::draw::Terrain *terrain;
        
    private:
        static void calculateAttributes(const tiny::draw::FloatTexture2D &, tiny::draw::RGBATexture2D &, const std::string &, const float &);
        static void applyUserAttributeMap(tiny::draw::RGBATexture2D &, const int &, const tiny::draw::RGBATexture2D &);
        
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
        
        tiny::vec2 scale;
        tiny::ivec2 farScale;
        tiny::vec2 farOffset;
        tiny::vec2 localTextureScale;
        
        std::string attributeShaderCode;
        std::list<std::pair<int, tiny::img::Image> > attributeMaps;
        
        tiny::draw::FloatTexture2D *heightTexture;
        tiny::draw::FloatTexture2D *farHeightTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        tiny::draw::RGBTexture2D *farNormalTexture;
        tiny::draw::RGBTexture2D *tangentTexture;
        tiny::draw::RGBTexture2D *farTangentTexture;
        tiny::draw::RGBATexture2D *attributeTexture;
        tiny::draw::RGBATexture2D *farAttributeTexture;
        tiny::draw::RGBTexture2DArray *localDiffuseTextures;
        tiny::draw::RGBTexture2DArray *localNormalTextures;
};

} //namespace moba

