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

#include <tiny/img/image.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/staticmeshhorde.h>

#include <tiny/snd/buffer.h>
#include <tiny/snd/source.h>

namespace tanks
{

struct ExplosionInstance
{
    ExplosionInstance(const unsigned int &a_type = 0) :
        type(a_type),
        x(0.0f),
        r(0.0f),
        sound(0)
    {

    }
    
    unsigned int type;
    tiny::vec3 x;
    float r;
    unsigned int sound;
};

struct ExplosionType
{
    public:
        ExplosionType(const std::string &, TiXmlElement *);
        ~ExplosionType();
        
        std::string name;
        float minRadius;
        float maxRadius;
        float expansionSpeed;
        float push;
        float damage;
        tiny::img::Image *explodeImage;
        tiny::snd::MonoSoundBuffer *explodeSound;
        tiny::vec4 icon;
};

struct BulletInstance
{
    BulletInstance(const unsigned int &a_type = 0, const unsigned int &a_explosionType = 0) :
        type(a_type),
        explosionType(a_explosionType),
        x(0.0f),
        v(0.0f),
        sound(0)
    {

    }
    
    unsigned int type;
    unsigned int explosionType;
    float lifetime;
    tiny::vec3 x;
    tiny::vec3 v;
    tiny::vec3 a;
    unsigned int sound;
};

class BulletType
{
    public:
        BulletType(const std::string &, TiXmlElement *);
        ~BulletType();
        
        std::string name;
        float radius;
        float lifetime;
        tiny::vec3 position;
        tiny::vec3 velocity;
        tiny::vec3 acceleration;
        tiny::img::Image *bulletImage;
        tiny::snd::MonoSoundBuffer *shootSound;
        tiny::snd::MonoSoundBuffer *travelSound;
        tiny::vec4 icon;
};

struct SoldierInstance
{
    SoldierInstance(const unsigned int &a_type = 0) :
        type(a_type),
        controls(0),
        angles(0.0f, 0.0f),
        x(0.0f),
        q(0.0f, 0.0f, 0.0f, 1.0f),
        P(0.0f),
        weaponRechargeTimes(),
        sound(0)
    {

    }
    
    unsigned int type;
    unsigned int controls;
    tiny::vec2 angles;
    tiny::vec3 x;
    tiny::vec4 q;
    tiny::vec3 P;
    std::vector<float> weaponRechargeTimes;
    unsigned int sound;
};

class SoldierWeapon
{
    public:
        SoldierWeapon(const std::string &, TiXmlElement *);
        ~SoldierWeapon();
        
        std::string name;
        float rechargeTime;
        std::string bulletName;
        unsigned int bulletType;
        std::string explosionName;
        unsigned int explosionType;
};

class SoldierType
{
    public:
        SoldierType(const std::string &, TiXmlElement *);
        ~SoldierType();
        
        void clearInstances();
        void addInstance(const SoldierInstance &);
        void updateInstances();
        
        tiny::vec3 getCameraPosition(const SoldierInstance &) const;
        tiny::vec4 getCameraOrientation(const SoldierInstance &) const;
        
        std::string name;
        float radius;
        float height;
        float mass;
        float jump;
        float speed;
        float landFriction;
        float airFriction;
        tiny::vec3 cameraPosition;
        
        bool jumpjet;
        float jumpjetThrust;
        float jumpjetFuel;
        float jumpjetCharge;
        
        std::vector<SoldierWeapon> weapons;
        
        tiny::draw::StaticMeshHorde *horde;
        tiny::draw::RGBTexture2D *diffuseTexture;
        tiny::draw::RGBTexture2D *normalTexture;
        int nrInstances;
        int maxNrInstances;
        std::vector<tiny::draw::StaticMeshInstance> instances;
};

} //namespace tanks

