/*
Copyright 2020, Bas Fagginger Auer.

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

#include <tiny/math/vec.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/staticmeshhorde.h>

namespace rpg
{

struct CharacterInstance
{
    CharacterInstance(const unsigned int &a_type = 0, const std::string &a_name = "", const tiny::ivec3 &a_position = tiny::ivec3(0), const int &a_rotation = 0, const int &a_state = 0, const float &a_color = 0.0f) :
        type(a_type),
        name(a_name),
        position(a_position),
        rotation(a_rotation),
        state(a_state),
        color(a_color)
    {
    
    }
    
    tiny::vec4 getColor() const
    {
        return CharacterInstance::palette(color);
    }

    static tiny::vec4 palette(const float &c)
    {
        return (c >= 0.0f ? tiny::vec4(0.5f*(1.0f + cosf(2.0f*static_cast<float>(M_PI)*(c + 0.0f/3.0f))),
                                0.5f*(1.0f + cosf(2.0f*static_cast<float>(M_PI)*(c + 1.0f/3.0f))),
                                0.5f*(1.0f + cosf(2.0f*static_cast<float>(M_PI)*(c + 2.0f/3.0f))),
                                1.0f) :
                                tiny::vec4(-c, -c, -c, 1.0f));    
    }
    
    unsigned int type;
    std::string name;
    tiny::ivec3 position;
    int rotation;
    int state;
    float color;
};

class CharacterType
{
    public:
        CharacterType(const std::string &, TiXmlElement *);
        ~CharacterType();
        
        void clearInstances();
        void addInstance(const CharacterInstance &, const float &);
        void updateInstances();
        
        std::string name;
        tiny::vec3 size;
        
        tiny::draw::StaticMeshHorde *shadowHorde;
        tiny::draw::RGBTexture2D *shadowDiffuseTexture;
        tiny::draw::RGBTexture2D *shadowNormalTexture;
        
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        
        int nrInstances;
        int maxNrInstances;
        std::vector<tiny::draw::StaticMeshInstance> instances;
        std::vector<tiny::draw::StaticMeshInstance> shadowInstances;
};

} //namespace rpg


