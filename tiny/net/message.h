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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <tiny/math/vec.h>

namespace tiny
{

namespace net
{

namespace vt
{

enum vt_enum
{
    Integer,
    IVec2,
    IVec3,
    IVec4,
    Float,
    Vec2,
    Vec3,
    Vec4,
    String
};

}

struct VariableType
{
    VariableType(const std::string &, const vt::vt_enum &);
    
    std::string name;
    vt::vt_enum type;
};

struct VariableData
{
    VariableData();
    VariableData(const int &);
    VariableData(const ivec2 &);
    VariableData(const ivec3 &);
    VariableData(const ivec4 &);
    VariableData(const float &);
    VariableData(const vec2 &);
    VariableData(const vec3 &);
    VariableData(const vec4 &);
    VariableData(const std::string &);
    
    int iv1;
    ivec2 iv2;
    ivec3 iv3;
    ivec4 iv4;
    float v1;
    vec2 v2;
    vec3 v3;
    vec4 v4;
    std::string s;
};

class MessageType
{
    public:
        MessageType(const std::string &, const std::string & = "");
        virtual ~MessageType();
        
        size_t getNrVariables() const;
        std::string getDescription() const;
        size_t getMinimumSizeInBytes() const;
        bool extractDataFromString(const std::string &, VariableData *) const;
        std::string convertDataToString(const VariableData *) const;
        
    protected:
        void addVariableType(const std::string &, const vt::vt_enum &);
        
    private:
        const std::string name;
        const std::string usage;
        std::vector<VariableType> variableTypes;
};

}

}

