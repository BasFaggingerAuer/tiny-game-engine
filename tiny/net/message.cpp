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
#include <iostream>
#include <sstream>

#include <tiny/net/message.h>

using namespace tiny::net;

VariableType::VariableType(const std::string &a_name, const vt::vt_enum &a_type) :
    name(a_name),
    type(a_type)
{

}

VariableData::VariableData() :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const int &a) :
    iv1(a),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const ivec2 &a) :
    iv1(0),
    iv2(a),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const ivec3 &a) :
    iv1(0),
    iv2(0, 0),
    iv3(a),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const ivec4 &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(a),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const float &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(a),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const vec2 &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(a),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const vec3 &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(a),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s("")
{

}

VariableData::VariableData(const vec4 &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(a),
    s("")
{

}

VariableData::VariableData(const std::string &a) :
    iv1(0),
    iv2(0, 0),
    iv3(0, 0, 0),
    iv4(0, 0, 0, 0),
    v1(0.0f),
    v2(0.0f, 0.0f),
    v3(0.0f, 0.0f, 0.0f),
    v4(0.0f, 0.0f, 0.0f, 0.0f),
    s(a)
{

}

MessageType::MessageType(const std::string &a_name, const std::string &a_usage) :
    name(a_name),
    usage(a_usage),
    variableTypes()
{

}

MessageType::~MessageType()
{

}

size_t MessageType::getNrVariables() const
{
    return variableTypes.size();
}

std::string MessageType::getDescription() const
{
    std::ostringstream stream;
    
    stream << name << "(";
    
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
             if (i->type == vt::Integer) stream << "int";
        else if (i->type == vt::IVec2) stream << "ivec2";
        else if (i->type == vt::IVec3) stream << "ivec3";
        else if (i->type == vt::IVec4) stream << "ivec4";
        else if (i->type == vt::Float) stream << "float";
        else if (i->type == vt::Vec2) stream << "vec2";
        else if (i->type == vt::Vec3) stream << "vec3";
        else if (i->type == vt::Vec4) stream << "vec4";
        else if (i->type == vt::String) stream << "string";
        
        stream << i->name << (i + 1 < variableTypes.end() ? "," : ")");
    }
    
    stream << std::endl << "Minimum size (bytes): " << getMinimumSizeInBytes() << std::endl;
    stream << std::endl << std::endl << usage;
    
    return stream.str();
}

size_t MessageType::getMinimumSizeInBytes() const
{
    //TODO: Include message type index?
    size_t length = 0;
    
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
             if (i->type == vt::Integer) length += sizeof(int);
        else if (i->type == vt::IVec2) length += sizeof(ivec2);
        else if (i->type == vt::IVec3) length += sizeof(ivec3);
        else if (i->type == vt::IVec4) length += sizeof(ivec4);
        else if (i->type == vt::Float) length += sizeof(float);
        else if (i->type == vt::Vec2) length += sizeof(vec2);
        else if (i->type == vt::Vec3) length += sizeof(vec3);
        else if (i->type == vt::Vec4) length += sizeof(vec4);
        else if (i->type == vt::String) length += 1;
    }
    
    return length;
}

std::string MessageType::convertDataToString(const VariableData *in) const
{
    std::ostringstream stream;
    
    stream << name;
    
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        stream << " ";
        
             if (i->type == vt::Integer) stream << in->iv1;
        else if (i->type == vt::IVec2) stream << in->iv2.x << " " << in->iv2.y;
        else if (i->type == vt::IVec3) stream << in->iv3.x << " " << in->iv3.y << " " << in->iv3.z;
        else if (i->type == vt::IVec4) stream << in->iv4.x << " " << in->iv4.y << " " << in->iv4.z << " " << in->iv4.w;
        else if (i->type == vt::Float) stream << in->v1;
        else if (i->type == vt::Vec2) stream << in->v2.x << " " << in->v2.y;
        else if (i->type == vt::Vec3) stream << in->v3.x << " " << in->v3.y << " " << in->v3.z;
        else if (i->type == vt::Vec4) stream << in->v4.x << " " << in->v4.y << " " << in->v4.z << " " << in->v4.w;
        else if (i->type == vt::String) stream << in->s;
        
        in++;
    }
    
    return stream.str();
}

bool MessageType::extractDataFromString(const std::string &in, VariableData *out) const
{
    std::istringstream stream(in);
    std::string messageName = "";
    
    //String should contain data for this particular message.
    stream >> messageName;
    
    if (messageName != name) return false;
    
    //Start extracting data.
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        if (i->type == vt::Integer)
        {
            int a = 0;
            
            stream >> a;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::IVec2)
        {
            ivec2 a(0, 0);
            
            stream >> a.x >> a.y;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::IVec3)
        {
            ivec3 a(0, 0, 0);
            
            stream >> a.x >> a.y >> a.z;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::IVec4)
        {
            ivec4 a(0, 0, 0, 0);
            
            stream >> a.x >> a.y >> a.z >> a.w;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::Float)
        {
            float a = 0.0f;
            
            stream >> a;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::Vec2)
        {
            vec2 a(0.0f, 0.0f);
            
            stream >> a.x >> a.y;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::Vec3)
        {
            vec3 a(0.0f, 0.0f, 0.0f);
            
            stream >> a.x >> a.y >> a.z;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::Vec4)
        {
            vec4 a(0.0f, 0.0f, 0.0f, 0.0f);
            
            stream >> a.x >> a.y >> a.z >> a.w;
            *out++ = VariableData(a);
        }
        else if (i->type == vt::String)
        {
            std::string a = "";
            
            stream >> a;
            *out++ = VariableData(a);
        }
    }
    
    return true;
}

void MessageType::addVariableType(const std::string &a_name, const vt::vt_enum &a_type)
{
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        if (i->name == a_name)
        {
            std::cerr << "Warning: Variable '" << a_name << "' occurs multiple times in messages of type '" << name << "'!" << std::endl;
        }
    }
    
    variableTypes.push_back(VariableType(a_name, a_type));
}


