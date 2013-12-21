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
#include <map>

#include <tinyxml.h>

#include <tiny/draw/texture2d.h>
#include <tiny/draw/staticmeshhorde.h>

namespace tanks
{

struct BulletInstance
{
    BulletInstance(const unsigned int &a_type = 0) :
        type(a_type),
        x(0.0f),
        v(0.0f)
    {

    }
    
    unsigned int type;
    tiny::vec3 x;
    tiny::vec3 v;
};

class BulletType
{
    public:
        BulletType(const std::string &, TiXmlElement *);
        ~BulletType();
        
};

struct SoldierInstance
{
    SoldierInstance(const unsigned int &a_type = 0) :
        type(a_type),
        controls(0),
        angles(0.0f, 0.0f),
        x(0.0f),
        q(0.0f, 0.0f, 0.0f, 1.0f),
        P(0.0f)
    {

    }
    
    unsigned int type;
    unsigned int controls;
    tiny::vec2 angles;
    tiny::vec3 x;
    tiny::vec4 q;
    tiny::vec3 P;
};

class SoldierType
{
    public:
        SoldierType(const std::string &, TiXmlElement *);
        ~SoldierType();
        
        void clearInstances();
        void addInstance(const SoldierInstance &);
        void updateInstances();
        
        std::string name;
        float radius;
        float height;
        float mass;
        float jump;
        float speed;
        tiny::vec3 cameraPosition;
        
        bool jumpjet;
        float jumpjetThrust;
        float jumpjetFuel;
        float jumpjetCharge;
        
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        int nrInstances;
        int maxNrInstances;
        std::vector<tiny::draw::StaticMeshInstance> instances;
};

} //namespace tanks

