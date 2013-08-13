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

struct TankInstance
{
    TankInstance(const unsigned int &a_type = 0) :
        type(a_type),
        controls(0),
        x(0.0f),
        q(0.0f, 0.0f, 0.0f, 1.0f),
        P(0.0f),
        L(0.0f)
    {

    }
    
    unsigned int type;
    unsigned int controls;
    tiny::vec3 x; //position
    tiny::vec4 q; //orientation
    tiny::vec3 P; //linear momentum
    tiny::vec3 L; //angular momentum
};

class TankType
{
    public:
        TankType(const std::string &, TiXmlElement *);
        ~TankType();
        
        void clearInstances();
        void addInstance(const TankInstance &);
        void updateInstances();
        
        std::string name;
        float mass;
        float inertia;
        float radius1;
        float radius2;
        tiny::vec3 thrust_pos[4];
        tiny::vec3 thrust_force[4];
        
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        int nrInstances;
        int maxNrInstances;
        std::vector<tiny::draw::StaticMeshInstance> instances;
};

} //namespace tanks

