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
#include <cassert>

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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(0.0f, 0.0f, 0.0f, 0.0f)
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
    v4(a)
{

}

MessageType::MessageType(const id_t &a_id, const std::string &a_name, const std::string &a_usage) :
    id(a_id),
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
    
    stream << id << ": " << name << "(";
    
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
        
        stream << " " << i->name << (i + 1 < variableTypes.end() ? "," : ")");
    }
    
    stream << std::endl << "Message size (bytes): " << getSizeInBytes() << std::endl;
    stream << std::endl << std::endl << usage;
    
    return stream.str();
}

size_t MessageType::getSizeInBytes() const
{
    size_t length = sizeof(id_t);
    
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
    }
    
    return length;
}

bool MessageType::messageToText(const Message &in, std::string &out) const
{
    if (in.id != id || in.data.size() != variableTypes.size())
    {
        std::cerr << "Error: Message has invalid id or the wrong number of variables!" << std::endl;
        return false;
    }
    
    std::ostringstream stream;
    const VariableData *ptr = &in.data[0];
    
    stream << name;
    
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        stream << " ";
        
             if (i->type == vt::Integer) stream << ptr->iv1;
        else if (i->type == vt::IVec2) stream << ptr->iv2.x << " " << ptr->iv2.y;
        else if (i->type == vt::IVec3) stream << ptr->iv3.x << " " << ptr->iv3.y << " " << ptr->iv3.z;
        else if (i->type == vt::IVec4) stream << ptr->iv4.x << " " << ptr->iv4.y << " " << ptr->iv4.z << " " << ptr->iv4.w;
        else if (i->type == vt::Float) stream << ptr->v1;
        else if (i->type == vt::Vec2) stream << ptr->v2.x << " " << ptr->v2.y;
        else if (i->type == vt::Vec3) stream << ptr->v3.x << " " << ptr->v3.y << " " << ptr->v3.z;
        else if (i->type == vt::Vec4) stream << ptr->v4.x << " " << ptr->v4.y << " " << ptr->v4.z << " " << ptr->v4.w;
        
        ptr++;
    }
    
    out = stream.str();
    
    return true;
}

bool MessageType::textToMessage(const std::string &in, Message &out) const
{
    std::istringstream stream(in);
    std::string messageName = "";
    
    //String should contain data for this particular message.
    stream >> messageName;
    
    if (messageName != name) return false;
    
    out.id = id;
    out.data.resize(variableTypes.size());
    
    VariableData *ptr = &out.data[0];
    
    //Start extracting data.
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        if (i->type == vt::Integer)
        {
            int a = 0;
            
            stream >> a;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::IVec2)
        {
            ivec2 a(0, 0);
            
            stream >> a.x >> a.y;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::IVec3)
        {
            ivec3 a(0, 0, 0);
            
            stream >> a.x >> a.y >> a.z;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::IVec4)
        {
            ivec4 a(0, 0, 0, 0);
            
            stream >> a.x >> a.y >> a.z >> a.w;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::Float)
        {
            float a = 0.0f;
            
            stream >> a;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::Vec2)
        {
            vec2 a(0.0f, 0.0f);
            
            stream >> a.x >> a.y;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::Vec3)
        {
            vec3 a(0.0f, 0.0f, 0.0f);
            
            stream >> a.x >> a.y >> a.z;
            *ptr++ = VariableData(a);
        }
        else if (i->type == vt::Vec4)
        {
            vec4 a(0.0f, 0.0f, 0.0f, 0.0f);
            
            stream >> a.x >> a.y >> a.z >> a.w;
            *ptr++ = VariableData(a);
        }
    }
    
    return true;
}

size_t MessageType::dataToMessage(const unsigned char *data, Message &out) const
{
    //TODO: Different-endian systems.
    const unsigned char *dataPtr = data;
    
    //Read identifier.
    const id_t dataId = *(const id_t *)dataPtr;
    
    dataPtr += sizeof(id_t);
    
    //Invalid id.
    if (dataId != id)
    {
        std::cerr << "Warning: Raw message with wrong data type, " << dataId << " instead of " << id << "!" << std::endl;
        return 0;
    }
    
    //Create new message.
    out.id = id;
    out.data.resize(variableTypes.size());
    
    VariableData *msgPtr = &out.data[0];
    
    //Start extracting data.
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        if (i->type == vt::Integer)
        {
            int a = *(const int *)dataPtr;
            
            dataPtr += sizeof(int);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::IVec2)
        {
            ivec2 a = *(const ivec2 *)dataPtr;
            
            dataPtr += sizeof(ivec2);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::IVec3)
        {
            ivec3 a = *(const ivec3 *)dataPtr;
            
            dataPtr += sizeof(ivec3);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::IVec4)
        {
            ivec4 a = *(const ivec4 *)dataPtr;
            
            dataPtr += sizeof(ivec4);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::Float)
        {
            float a = *(const float *)dataPtr;
            
            dataPtr += sizeof(float);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::Vec2)
        {
            vec2 a = *(const vec2 *)dataPtr;
            
            dataPtr += sizeof(vec2);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::Vec3)
        {
            vec3 a = *(const vec3 *)dataPtr;
            
            dataPtr += sizeof(vec3);
            *msgPtr++ = VariableData(a);
        }
        else if (i->type == vt::Vec4)
        {
            vec4 a = *(const vec4 *)dataPtr;
            
            dataPtr += sizeof(vec4);
            *msgPtr++ = VariableData(a);
        }
    }
    
    assert(dataPtr - data == static_cast<int>(getSizeInBytes()));
    
    return dataPtr - data;
}

size_t MessageType::messageToData(const Message &in, unsigned char *out) const
{
    if (in.id != id || in.data.size() != variableTypes.size())
    {
        std::cerr << "Error: Message has invalid id or the wrong number of variables!" << std::endl;
        return 0;
    }
    
    const VariableData *msgPtr = &in.data[0];
    unsigned char *dataPtr = out;
    
    *(id_t *)dataPtr = id;
    dataPtr += sizeof(id_t);
    
    for (std::vector<VariableType>::const_iterator i = variableTypes.begin(); i != variableTypes.end(); ++i)
    {
        if (i->type == vt::Integer)
        {
            *(int *)dataPtr = msgPtr->iv1;
            dataPtr += sizeof(int);
        }
        else if (i->type == vt::IVec2)
        {
            *(ivec2 *)dataPtr = msgPtr->iv2;
            dataPtr += sizeof(ivec2);
        }
        else if (i->type == vt::IVec3)
        {
            *(ivec3 *)dataPtr = msgPtr->iv3;
            dataPtr += sizeof(ivec3);
        }
        else if (i->type == vt::IVec4)
        {
            *(ivec4 *)dataPtr = msgPtr->iv4;
            dataPtr += sizeof(ivec4);
        }
        if (i->type == vt::Float)
        {
            *(float *)dataPtr = msgPtr->v1;
            dataPtr += sizeof(float);
        }
        else if (i->type == vt::Vec2)
        {
            *(vec2 *)dataPtr = msgPtr->v2;
            dataPtr += sizeof(vec2);
        }
        else if (i->type == vt::Vec3)
        {
            *(vec3 *)dataPtr = msgPtr->v3;
            dataPtr += sizeof(vec3);
        }
        else if (i->type == vt::Vec4)
        {
            *(vec4 *)dataPtr = msgPtr->v4;
            dataPtr += sizeof(vec4);
        }
        
        msgPtr++;
    }
    
    assert(dataPtr - out == static_cast<int>(getSizeInBytes()));
    
    return dataPtr - out;
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

MessageTranslator::MessageTranslator() :
    messageBuffer(65536)
{

}

MessageTranslator::~MessageTranslator()
{

}

bool MessageTranslator::addMessageType(const MessageType *type)
{
    if (!type)
    {
        std::cerr << "Warning: Null message type!" << std::endl;
        return false;
    }
    
    if (messageTypes.find(type->id) != messageTypes.end())
    {
        std::cerr << "Warning: The message type '" << type->name << "' with id " << type->id << " already exists in this translator!" << std::endl;
        return false;
    }
    
    messageTypes[type->id] = type;
    
    return true;
}

bool MessageTranslator::textToMessage(const std::string &text, Message &message) const
{
    //Retrieve message name.
    std::istringstream stream(text);
    std::string messageName = "";
    
    stream >> messageName;
    
    for (std::map<id_t, const MessageType *>::const_iterator i = messageTypes.begin(); i != messageTypes.end(); ++i)
    {
        if (i->second->name == messageName)
        {
            return i->second->textToMessage(text, message);
        }
    }
    
    std::cerr << "Unable to find message for '" << text << "'!" << std::endl;
    
    return false;
}

bool MessageTranslator::messageToText(const Message &message, std::string &text) const
{
    std::map<id_t, const MessageType *>::const_iterator i = messageTypes.find(message.id);
    
    if (i == messageTypes.end())
    {
        std::cerr << "Unable to find message type with id " << message.id << "!" << std::endl;
        return false;
    }
    
    return i->second->messageToText(message, text);
}

bool MessageTranslator::sendMessageTCP(const Message &message, TCPsocket socket)
{
    std::map<id_t, const MessageType *>::const_iterator i = messageTypes.find(message.id);
    
    if (i == messageTypes.end())
    {
        std::cerr << "Unable to find message type with id " << message.id << "!" << std::endl;
        return false;
    }
    
    const size_t messageSize = i->second->getSizeInBytes();
    
    if (messageSize > messageBuffer.size() || messageSize == 0)
    {
        std::cerr << "Message is too large to be sent over TCP (" << messageSize << " bytes)!" << std::endl;
        return false;
    }
    
    if (i->second->messageToData(message, &messageBuffer[0]) != messageSize)
    {
        std::cerr << "Unable to convert message to raw data!" << std::endl;
        return false;
    }
    
    if (SDLNet_TCP_Send(socket, &messageBuffer[0], messageSize) != static_cast<int>(messageSize))
    {
        std::cerr << "Unable to send all data over TCP: " << SDLNet_GetError() << "!" << std::endl;
        return false;
    }
    
    return true;
}

bool MessageTranslator::receiveMessageTCP(TCPsocket socket, Message &message)
{
    //Receive id.
    if (SDLNet_TCP_Recv(socket, &messageBuffer[0], sizeof(id_t)) != sizeof(id_t))
    {
        std::cerr << "Unable to receive message id over TCP!" << std::endl;
        return false;
    }
    
    const id_t id = *(id_t *)&messageBuffer[0];
    std::map<id_t, const MessageType *>::const_iterator i = messageTypes.find(id);
    
    if (i == messageTypes.end())
    {
        std::cerr << "Unable to find message type with id " << id << "!" << std::endl;
        return false;
    }
    
    const size_t messageSize = i->second->getSizeInBytes();
    
    if (messageSize > messageBuffer.size() || messageSize == 0)
    {
        std::cerr << "Message is too large to be sent over TCP (" << messageSize << " bytes)!" << std::endl;
        return false;
    }
    
    //Receive data.
    if (SDLNet_TCP_Recv(socket, &messageBuffer[sizeof(id_t)], messageSize - sizeof(id_t)) != static_cast<int>(messageSize - sizeof(id_t)))
    {
        std::cerr << "Unable to receive message contents over TCP!" << std::endl;
        return false;
    }
    
    if (i->second->dataToMessage(&messageBuffer[0], message) != messageSize)
    {
        std::cerr << "Unable to convert raw data to message!" << std::endl;
        return false;
    }
    
    return true;
}

