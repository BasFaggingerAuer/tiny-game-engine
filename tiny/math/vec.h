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
#include <cmath>

#ifdef ENABLE_OPENVR
#include <openvr.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace tiny
{

//Why does C++ not offer a mod()-operation that is >= 0?
//From https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c.
template <typename t>
inline t modnonneg(const t &a, const t &b) {return ((a % b) + b) % b;}

template <typename t> class typed2vector
{
    public:
        inline typed2vector() : x(0), y(0) {};
        inline typed2vector(const t &a) : x(a), y(a) {};
        inline typed2vector(const t &_x, const t &_y) : x(_x), y(_y) {};
        inline typed2vector(const typed2vector<t> &a) : x(a.x), y(a.y) {};
        inline ~typed2vector() {};
        
        inline typed2vector & operator = (const typed2vector<t> &a) {x = a.x; y = a.y; return *this;};
        inline bool operator == (const typed2vector<t> &a) const {return (a.x == x && a.y == y);}
        inline bool operator != (const typed2vector<t> &a) const {return (a.x != x || a.y != y);}
        inline typed2vector & operator += (const typed2vector<t> &a) {x += a.x; y += a.y; return *this;};
        inline typed2vector & operator += (const t &a) {x += a; y += a; return *this;};
        inline typed2vector & operator -= (const typed2vector<t> &a) {x -= a.x; y -= a.y; return *this;};
        inline typed2vector & operator -= (const t &a) {x -= a; y -= a; return *this;};
        inline typed2vector & operator *= (const typed2vector<t> &a) {x *= a.x; y *= a.y; return *this;};
        inline typed2vector & operator /= (const typed2vector<t> &a) {x /= a.x; y /= a.y; return *this;};
        inline typed2vector & operator *= (const t &a) {x *= a; y *= a; return *this;};
        inline typed2vector & operator /= (const t &a) {x /= a; y /= a; return *this;};
        inline t dot(const typed2vector<t> &a) const {return x*a.x + y*a.y;};
        
        friend std::ostream & operator << (std::ostream &Out, const typed2vector<t> &a) {Out << "(" << a.x << ", " << a.y << ")"; return Out;};
        
        t x, y;
};

template <typename t> class typed4vector;

template <typename t> class typed3vector
{
    public:
        inline typed3vector() : x(0), y(0), z(0) {};
        inline typed3vector(const t &a) : x(a), y(a), z(a) {};
        inline typed3vector(const t &_x, const t &_y, const t &_z) : x(_x), y(_y), z(_z) {};
        inline typed3vector(const typed2vector<t> &a, const t &_z) : x(a.x), y(a.y), z(_z) {};
        inline typed3vector(const typed3vector<t> &a) : x(a.x), y(a.y), z(a.z) {};
        inline ~typed3vector() {};

        inline typed3vector & operator = (const typed3vector<t> &a) {x = a.x; y = a.y; z = a.z; return *this;};
        inline bool operator == (const typed3vector<t> &a) const {return (a.x == x && a.y == y && a.z == z);}
        inline bool operator != (const typed3vector<t> &a) const {return (a.x != x || a.y != y || a.z != z);}
        inline typed3vector & operator += (const typed3vector<t> &a) {x += a.x; y += a.y; z += a.z; return *this;};
        inline typed3vector & operator += (const t &a) {x += a; y += a; z += a; return *this;};
        inline typed3vector & operator -= (const typed3vector<t> &a) {x -= a.x; y -= a.y; z -= a.z; return *this;};
        inline typed3vector & operator -= (const t &a) {x -= a; y -= a; z -= a; return *this;};
        inline typed3vector & operator *= (const typed3vector<t> &a) {x *= a.x; y *= a.y; z *= a.z; return *this;};
        inline typed3vector & operator /= (const typed3vector<t> &a) {x /= a.x; y /= a.y; z /= a.z; return *this;};
        inline typed3vector & operator *= (const t &a) {x *= a; y *= a; z *= a; return *this;};
        inline typed3vector & operator /= (const t &a) {x /= a; y /= a; z /= a; return *this;};
        inline t dot(const typed3vector<t> &a) const {return x*a.x + y*a.y + z*a.z;};
        
        inline typed2vector<t> xy() const {return typed2vector<t>(x, y);};
        
        friend std::ostream & operator << (std::ostream &Out, const typed3vector<t> &a) {Out << "(" << a.x << ", " << a.y << ", " << a.z << ")"; return Out;};
        
        t x, y, z;
};

template <typename t> class typed4vector
{
    public:
        inline typed4vector() : x(0), y(0), z(0), w(0) {};
        inline typed4vector(const t &a) : x(a), y(a), z(a), w(a) {};
        inline typed4vector(const t &_x, const t &_y, const t &_z, const t &_w) : x(_x), y(_y), z(_z), w(_w) {};
        inline typed4vector(const typed2vector<t> &a, const t &_z, const t &_w) : x(a.x), y(a.y), z(_z), w(_w) {};
        inline typed4vector(const typed3vector<t> &a, const t &_w) : x(a.x), y(a.y), z(a.z), w(_w) {};
        inline typed4vector(const typed4vector<t> &a) : x(a.x), y(a.y), z(a.z), w(a.w) {};
        inline ~typed4vector() {};
        
        inline typed4vector & operator = (const typed4vector<t> &a) {x = a.x; y = a.y; z = a.z; w = a.w; return *this;};
        inline bool operator == (const typed4vector<t> &a) const {return (a.x == x && a.y == y && a.z == z && a.w == w);}
        inline bool operator != (const typed4vector<t> &a) const {return (a.x != x || a.y != y || a.z != z || a.w != w);}
        inline typed4vector & operator += (const typed4vector<t> &a) {x += a.x; y += a.y; z += a.z; w += a.w; return *this;};
        inline typed4vector & operator += (const t &a) {x += a; y += a; z += a; w += a; return *this;};
        inline typed4vector & operator -= (const typed4vector<t> &a) {x -= a.x; y -= a.y; z -= a.z; w -= a.w; return *this;};
        inline typed4vector & operator -= (const t &a) {x -= a; y -= a; z -= a; w -= a; return *this;};
        inline typed4vector & operator *= (const typed4vector<t> &a) {x *= a.x; y *= a.y; z *= a.z; w *= a.w; return *this;};
        inline typed4vector & operator /= (const typed4vector<t> &a) {x /= a.x; y /= a.y; z /= a.z; w /= a.w; return *this;};
        inline typed4vector & operator *= (const t &a) {x *= a; y *= a; z *= a; w *= a; return *this;};
        inline typed4vector & operator /= (const t &a) {x /= a; y /= a; z /= a; w /= a; return *this;};
        inline t dot(const typed4vector<t> &a) const {return x*a.x + y*a.y + z*a.z + w*a.w;};
        
        inline typed2vector<t> xy() const {return typed2vector<t>(x, y);};
        inline typed3vector<t> xyz() const {return typed3vector<t>(x, y, z);};
        
        friend std::ostream & operator << (std::ostream &Out, const typed4vector<t> &a) {Out << "(" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << ")"; return Out;};
        
        t x, y, z, w;
};

template <typename t> typed2vector<t> & operator + (typed2vector<t> a, const typed2vector<t> &b) {return a += b;}
template <typename t> typed2vector<t> & operator + (typed2vector<t> a, const t &b) {return a += b;}
template <typename t> typed2vector<t> & operator - (typed2vector<t> a, const typed2vector<t> &b) {return a -= b;}
template <typename t> typed2vector<t> & operator - (typed2vector<t> a, const t &b) {return a -= b;}
template <typename t> typed2vector<t> & operator * (typed2vector<t> a, const typed2vector<t> &b) {return a *= b;}
template <typename t> typed2vector<t> & operator / (typed2vector<t> a, const typed2vector<t> &b) {return a /= b;}
template <typename t> typed2vector<t> & operator * (typed2vector<t> a, const t &b) {return a *= b;}
template <typename t> typed2vector<t> & operator * (const t &b, typed2vector<t> a) {return a *= b;}
template <typename t> typed2vector<t> & operator / (typed2vector<t> a, const t &b) {return a /= b;}
template <typename t> typed2vector<t> & operator / (const t &b, typed2vector<t> a) {return a /= b;}

template <typename t> typed3vector<t> & operator + (typed3vector<t> a, const typed3vector<t> &b) {return a += b;}
template <typename t> typed3vector<t> & operator + (typed3vector<t> a, const t &b) {return a += b;}
template <typename t> typed3vector<t> & operator - (typed3vector<t> a, const typed3vector<t> &b) {return a -= b;}
template <typename t> typed3vector<t> & operator - (typed3vector<t> a, const t &b) {return a -= b;}
template <typename t> typed3vector<t> & operator * (typed3vector<t> a, const typed3vector<t> &b) {return a *= b;}
template <typename t> typed3vector<t> & operator / (typed3vector<t> a, const typed3vector<t> &b) {return a /= b;}
template <typename t> typed3vector<t> & operator * (typed3vector<t> a, const t &b) {return a *= b;}
template <typename t> typed3vector<t> & operator * (const t &b, typed3vector<t> a) {return a *= b;}
template <typename t> typed3vector<t> & operator / (typed3vector<t> a, const t &b) {return a /= b;}
template <typename t> typed3vector<t> & operator / (const t &b, typed3vector<t> a) {return a /= b;}

template <typename t> typed4vector<t> & operator + (typed4vector<t> a, const typed4vector<t> &b) {return a += b;}
template <typename t> typed4vector<t> & operator + (typed4vector<t> a, const t &b) {return a += b;}
template <typename t> typed4vector<t> & operator - (typed4vector<t> a, const typed4vector<t> &b) {return a -= b;}
template <typename t> typed4vector<t> & operator - (typed4vector<t> a, const t &b) {return a -= b;}
template <typename t> typed4vector<t> & operator * (typed4vector<t> a, const typed4vector<t> &b) {return a *= b;}
template <typename t> typed4vector<t> & operator / (typed4vector<t> a, const typed4vector<t> &b) {return a /= b;}
template <typename t> typed4vector<t> & operator * (typed4vector<t> a, const t &b) {return a *= b;}
template <typename t> typed4vector<t> & operator * (const t &b, typed4vector<t> a) {return a *= b;}
template <typename t> typed4vector<t> & operator / (typed4vector<t> a, const t &b) {return a /= b;}
template <typename t> typed4vector<t> & operator / (const t &b, typed4vector<t> a) {return a /= b;}

template <typename t> typed2vector<t> min(const typed2vector<t> &a, const typed2vector<t> &b) {return typed2vector<t>(std::min(a.x, b.x), std::min(a.y, b.y));}
template <typename t> typed2vector<t> max(const typed2vector<t> &a, const typed2vector<t> &b) {return typed2vector<t>(std::max(a.x, b.x), std::max(a.y, b.y));}
template <typename t> typed3vector<t> min(const typed3vector<t> &a, const typed3vector<t> &b) {return typed3vector<t>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));}
template <typename t> typed3vector<t> max(const typed3vector<t> &a, const typed3vector<t> &b) {return typed3vector<t>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));}
template <typename t> typed4vector<t> min(const typed4vector<t> &a, const typed4vector<t> &b) {return typed4vector<t>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));}
template <typename t> typed4vector<t> max(const typed4vector<t> &a, const typed4vector<t> &b) {return typed4vector<t>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w));}

template <typename t> t clamp(const t &a, const t &b, const t &c) {return (a < b ? b : (a > c ? c : a));}

template <typename t> t dot(const typed2vector<t> &a, const typed2vector<t> &b) {return a.x*b.x + a.y*b.y;}
template <typename t> t dot(const typed3vector<t> &a, const typed3vector<t> &b) {return a.x*b.x + a.y*b.y + a.z*b.z;}
template <typename t> t dot(const typed4vector<t> &a, const typed4vector<t> &b) {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}

template <typename t> typed3vector<t> cross(const typed3vector<t> &a, const typed3vector<t> &b) {return typed3vector<t>(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);}

template <typename t> t length(const typed2vector<t> &a) {return sqrtf(dot(a, a));}
template <typename t> t length(const typed3vector<t> &a) {return sqrtf(dot(a, a));}
template <typename t> t length(const typed4vector<t> &a) {return sqrtf(dot(a, a));}

template <typename t> t length2(const typed2vector<t> &a) {return dot(a, a);}
template <typename t> t length2(const typed3vector<t> &a) {return dot(a, a);}
template <typename t> t length2(const typed4vector<t> &a) {return dot(a, a);}

template <typename t> typed2vector<t> normalize(const typed2vector<t> &a) {t l = dot(a, a); l = (l > static_cast<t>(1.0e-8) ? static_cast<t>(1.0/sqrt(l)) : static_cast<t>(1.0)); return typed2vector<t>(a.x*l, a.y*l);}
template <typename t> typed3vector<t> normalize(const typed3vector<t> &a) {t l = dot(a, a); l = (l > static_cast<t>(1.0e-8) ? static_cast<t>(1.0/sqrt(l)) : static_cast<t>(1.0)); return typed3vector<t>(a.x*l, a.y*l, a.z*l);}
template <typename t> typed4vector<t> normalize(const typed4vector<t> &a) {t l = dot(a, a); l = (l > static_cast<t>(1.0e-8) ? static_cast<t>(1.0/sqrt(l)) : static_cast<t>(1.0)); return typed4vector<t>(a.x*l, a.y*l, a.z*l, a.w*l);}

typedef typed2vector<int> ivec2;
typedef typed2vector<float> vec2;
typedef typed3vector<int> ivec3;
typedef typed3vector<float> vec3;
typedef typed4vector<int> ivec4;
typedef typed4vector<float> vec4;

inline vec3 & operator * (vec3 a, const float b) {return a *= b;}

//Integer specific bit operations.
inline ivec2 operator % (const ivec2 &a, const ivec2 &b) {return ivec2(a.x % b.x, a.y % b.y);}
inline ivec2 operator % (const ivec2 &a, const int &b) {return ivec2(a.x % b, a.y % b);}
inline ivec2 operator & (const ivec2 &a, const ivec2 &b) {return ivec2(a.x & b.x, a.y & b.y);}
inline ivec2 operator & (const ivec2 &a, const int &b) {return ivec2(a.x & b, a.y & b);}
inline ivec2 operator | (const ivec2 &a, const ivec2 &b) {return ivec2(a.x | b.x, a.y | b.y);}
inline ivec2 operator | (const ivec2 &a, const int &b) {return ivec2(a.x | b, a.y | b);}
inline ivec2 operator >> (const ivec2 &a, const ivec2 &b) {return ivec2(a.x >> b.x, a.y >> b.y);}
inline ivec2 operator >> (const ivec2 &a, const int &b) {return ivec2(a.x >> b, a.y >> b);}
inline ivec2 operator << (const ivec2 &a, const ivec2 &b) {return ivec2(a.x << b.x, a.y << b.y);}
inline ivec2 operator << (const ivec2 &a, const int &b) {return ivec2(a.x << b, a.y << b);}
inline bool operator < (const ivec2 &a, const ivec2 &b) {return (a.x == b.x ? a.y < b.y : a.x < b.x);}

inline ivec2 vfloor(const vec2 &a) {return ivec2(static_cast<int>(floor(a.x)), static_cast<int>(floor(a.y)));}
inline ivec2 vceil(const vec2 &a) {return ivec2(static_cast<int>(ceil(a.x)), static_cast<int>(ceil(a.y)));}
inline ivec3 vfloor(const vec3 &a) {return ivec3(static_cast<int>(floor(a.x)), static_cast<int>(floor(a.y)), static_cast<int>(floor(a.z)));}
inline ivec3 vceil(const vec3 &a) {return ivec3(static_cast<int>(ceil(a.x)), static_cast<int>(ceil(a.y)), static_cast<int>(ceil(a.z)));}
inline ivec4 vfloor(const vec4 &a) {return ivec4(static_cast<int>(floor(a.x)), static_cast<int>(floor(a.y)), static_cast<int>(floor(a.z)), static_cast<int>(floor(a.w)));}
inline ivec4 vceil(const vec4 &a) {return ivec4(static_cast<int>(ceil(a.x)), static_cast<int>(ceil(a.y)), static_cast<int>(ceil(a.z)), static_cast<int>(ceil(a.w)));}

class mat3
{
    public:
        inline mat3() {};
        inline mat3(const float &a) :
            v00(a), v10(a), v20(a),
            v01(a), v11(a), v21(a),
            v02(a), v12(a), v22(a)
        {};
        inline mat3(const mat3 &a) :
            v00(a.v00), v10(a.v10), v20(a.v20),
            v01(a.v01), v11(a.v11), v21(a.v21),
            v02(a.v02), v12(a.v12), v22(a.v22)
        {};
        inline mat3(const float &_v00, const float &_v10, const float &_v20,
                const float &_v01, const float &_v11, const float &_v21,
                const float &_v02, const float &_v12, const float &_v22) :
            v00(_v00), v10(_v10), v20(_v20),
            v01(_v01), v11(_v11), v21(_v21),
            v02(_v02), v12(_v12), v22(_v22)
        {};
        
        inline mat3(const vec3 &x, const vec3 &y, const vec3 &z) :
            v00(x.x), v10(x.y), v20(x.z),
            v01(y.x), v11(y.y), v21(y.z),
            v02(z.x), v12(z.y), v22(z.z)
        {};
        
        inline mat3(const vec4 &a) :
            v00(1.0f - 2.0f*(a.y*a.y + a.z*a.z)),
            v10(2.0f*(a.x*a.y - a.w*a.z)),
            v20(2.0f*(a.x*a.z + a.w*a.y)),
            v01(2.0f*(a.x*a.y + a.w*a.z)),
            v11(1.0f - 2.0f*(a.x*a.x + a.z*a.z)),
            v21(2.0f*(a.y*a.z - a.w*a.x)),
            v02(2.0f*(a.x*a.z - a.w*a.y)),
            v12(2.0f*(a.y*a.z + a.w*a.x)),
            v22(1.0f - 2.0f*(a.x*a.x + a.y*a.y))
        {};

        inline ~mat3() {};

        inline mat3 & operator = (const mat3 &b)
        {
            v00 = b.v00;
            v10 = b.v10;
            v20 = b.v20;
            v01 = b.v01;
            v11 = b.v11;
            v21 = b.v21;
            v02 = b.v02;
            v12 = b.v12;
            v22 = b.v22;
            return *this;
        };

        inline mat3 & operator += (const mat3 &b)
        {
            v00 += b.v00;
            v10 += b.v10;
            v20 += b.v20;
            v01 += b.v01;
            v11 += b.v11;
            v21 += b.v21;
            v02 += b.v02;
            v12 += b.v12;
            v22 += b.v22;
            return *this;
        };

        inline mat3 & operator -= (const mat3 &b)
        {
            v00 -= b.v00;
            v10 -= b.v10;
            v20 -= b.v20;
            v01 -= b.v01;
            v11 -= b.v11;
            v21 -= b.v21;
            v02 -= b.v02;
            v12 -= b.v12;
            v22 -= b.v22;
            return *this;
        };

        inline mat3 & operator *= (const mat3 &b)
        {
            const mat3 c(
                v00*b.v00 + v01*b.v10 + v02*b.v20,
                v10*b.v00 + v11*b.v10 + v12*b.v20,
                v20*b.v00 + v21*b.v10 + v22*b.v20,
                
                v00*b.v01 + v01*b.v11 + v02*b.v21,
                v10*b.v01 + v11*b.v11 + v12*b.v21,
                v20*b.v01 + v21*b.v11 + v22*b.v21,
                
                v00*b.v02 + v01*b.v12 + v02*b.v22,
                v10*b.v02 + v11*b.v12 + v12*b.v22,
                v20*b.v02 + v21*b.v12 + v22*b.v22);
            *this = c;
            return *this;
        };

        inline mat3 & operator *= (const float &b)
        {
            v00 *= b;
            v10 *= b;
            v20 *= b;
            v01 *= b;
            v11 *= b;
            v21 *= b;
            v02 *= b;
            v12 *= b;
            v22 *= b;
            return *this;
        };

        inline vec3 operator * (const vec3 &a) const
        {
            return vec3(v00*a.x + v01*a.y + v02*a.z,
                    v10*a.x + v11*a.y + v12*a.z,
                    v20*a.x + v21*a.y + v22*a.z);
        };

        inline void toOpenGL(float * const v) const
        {
            v[ 0] = v00;
            v[ 1] = v10;
            v[ 2] = v20;
            v[ 3] = 0.0f;
            v[ 4] = v01;
            v[ 5] = v11;
            v[ 6] = v21;
            v[ 7] = 0.0f;
            v[ 8] = v02;
            v[ 9] = v12;
            v[10] = v22;
            v[11] = 0.0f;
            v[12] = 0.0f;
            v[13] = 0.0f;
            v[14] = 0.0f;
            v[15] = 1.0f;
        };
        
        inline mat3 transposed() const
        {
            return mat3(v00, v01, v02,
                        v10, v11, v12,
                        v20, v21, v22);
        };
        
        inline mat3 inverted() const
        {
            const float t = 1.0f/(v00*(v11*v22 - v12*v21) - v01*(v10*v22 - v12*v20) + v02*(v10*v21 - v11*v20));

            return mat3(
                    t*(v11*v22 - v12*v21), t*(v12*v20 - v10*v22), t*(v10*v21 - v11*v20),
                    t*(v02*v21 - v01*v22), t*(v00*v22 - v02*v20), t*(v01*v20 - v00*v21),
                    t*(v01*v12 - v02*v11), t*(v02*v10 - v00*v12), t*(v00*v11 - v01*v10));
        };

        inline vec4 getRotation() const
        {
            //From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm.
            const float t = v00 + v11 + v22;
            vec4 q;
            
            if (t > 0.0f)
            {
                const float s = 0.5f/sqrtf(1.0f + t);
                
                q = vec4((v21 - v12)*s, (v02 - v20)*s, (v10 - v01)*s, 0.25f/s);
            }
            else
            {
                if (v00 > v11 && v00 > v22)
                {
                    const float s = 0.5f/sqrtf(1.0f + v00 - v11 - v22);
                    
                    q = vec4(0.25f/s, (v01 + v10)*s, (v02 + v20)*s, (v21 - v12)*s);
                }
                else if (v11 > v22)
                {
                    const float s = 0.5f/sqrtf(1.0f + v11 - v00 - v22);
                    
                    q = vec4((v01 + v10)*s, 0.25f/s, (v12 + v21)*s, (v02 - v20)*s);
                }
                else
                {
                    const float s = 0.5f/sqrtf(1.0f + v22 - v00 - v11);
                    
                    q = vec4((v02 + v20)*s, (v12 + v21)*s, 0.25f/s, (v10 - v01)*s);
                }
            }
            
            return normalize(q);
        };
        
        static mat3 identityMatrix()
        {
            return mat3(1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0,
                        0.0, 0.0, 1.0);
        };

        static mat3 outerProductMatrix(const vec3 &a, const vec3 &b)
        {
            return mat3(a.x*b.x, a.y*b.x, a.z*b.x,
                        a.x*b.y, a.y*b.y, a.z*b.y,
                        a.x*b.z, a.y*b.z, a.z*b.z);
        };
        
        static mat3 scaleMatrix(const vec3 &a)
        {
            return mat3(a.x, 0.0, 0.0,
                        0.0, a.y, 0.0,
                        0.0, 0.0, a.z);
        };
        
        static mat3 rotationMatrix(const vec4 &a)
        {
            return mat3(
                1.0f - 2.0f*(a.y*a.y + a.z*a.z),
                2.0f*(a.x*a.y - a.w*a.z),
                2.0f*(a.x*a.z + a.w*a.y),
                2.0f*(a.x*a.y + a.w*a.z),
                1.0f - 2.0f*(a.x*a.x + a.z*a.z),
                2.0f*(a.y*a.z - a.w*a.x),
                2.0f*(a.x*a.z - a.w*a.y),
                2.0f*(a.y*a.z + a.w*a.x),
                1.0f - 2.0f*(a.x*a.x + a.y*a.y));
        };
        
        float v00, v10, v20;
        float v01, v11, v21;
        float v02, v12, v22;
        
        friend std::ostream & operator << (std::ostream &Out, const mat3 &a)
        {
            Out << a.v00 << ", " << a.v01 << ", " << a.v02 << std::endl
                << a.v10 << ", " << a.v11 << ", " << a.v12 << std::endl
                << a.v20 << ", " << a.v21 << ", " << a.v22 << std::endl;
            return Out;
        };
};

inline mat3 & operator + (mat3 a, const mat3 &b) {return a += b;}
inline mat3 & operator - (mat3 a, const mat3 &b) {return a -= b;}
inline mat3 & operator * (mat3 a, const mat3 &b) {return a *= b;}
inline mat3 & operator * (const float &b, mat3 a) {return a *= b;}

class mat4
{
    public:
        inline mat4() {};
        inline mat4(const float &a) :
            v00(a), v10(a), v20(a), v30(a),
            v01(a), v11(a), v21(a), v31(a),
            v02(a), v12(a), v22(a), v32(a),
            v03(a), v13(a), v23(a), v33(a)
        {};
        inline mat4(const mat4 &a) :
            v00(a.v00), v10(a.v10), v20(a.v20), v30(a.v30),
            v01(a.v01), v11(a.v11), v21(a.v21), v31(a.v31),
            v02(a.v02), v12(a.v12), v22(a.v22), v32(a.v32),
            v03(a.v03), v13(a.v13), v23(a.v23), v33(a.v33)
        {};
        inline mat4(const float &_v00, const float &_v10, const float &_v20, const float &_v30,
                const float &_v01, const float &_v11, const float &_v21, const float &_v31,
                const float &_v02, const float &_v12, const float &_v22, const float &_v32,
                const float &_v03, const float &_v13, const float &_v23, const float &_v33) :
            v00(_v00), v10(_v10), v20(_v20), v30(_v30),
            v01(_v01), v11(_v11), v21(_v21), v31(_v31),
            v02(_v02), v12(_v12), v22(_v22), v32(_v32),
            v03(_v03), v13(_v13), v23(_v23), v33(_v33)
        {};

        inline mat4(const mat3 &a) :
            v00(a.v00), v10(a.v10), v20(a.v20), v30(0.0f),
            v01(a.v01), v11(a.v11), v21(a.v21), v31(0.0f),
            v02(a.v02), v12(a.v12), v22(a.v22), v32(0.0f),
            v03(0.0f),  v13(0.0f),  v23(0.0f),  v33(1.0f)
        {};
        
        inline mat4(const vec3 &x, const vec3 &y, const vec3 &z, const vec3 &b = vec3(0.0f, 0.0f, 0.0f)) :
            v00(x.x), v10(x.y), v20(x.z), v30(0.0f),
            v01(y.x), v11(y.y), v21(y.z), v31(0.0f),
            v02(z.x), v12(z.y), v22(z.z), v32(0.0f),
            v03(b.x), v13(b.y), v23(b.z), v33(1.0f)
        {};
        
        inline mat4(const vec4 &a, const vec3 &b = vec3(0.0f, 0.0f, 0.0f)) :
            v00(1.0f - 2.0f*(a.y*a.y + a.z*a.z)),
            v10(2.0f*(a.x*a.y - a.w*a.z)),
            v20(2.0f*(a.x*a.z + a.w*a.y)),
            v30(0.0f),
            v01(2.0f*(a.x*a.y + a.w*a.z)),
            v11(1.0f - 2.0f*(a.x*a.x + a.z*a.z)),
            v21(2.0f*(a.y*a.z - a.w*a.x)),
            v31(0.0f),
            v02(2.0f*(a.x*a.z - a.w*a.y)),
            v12(2.0f*(a.y*a.z + a.w*a.x)),
            v22(1.0f - 2.0f*(a.x*a.x + a.y*a.y)),
            v32(0.0f),
            v03(b.x),
            v13(b.y),
            v23(b.z),
            v33(1.0f)
        {};

#ifdef ENABLE_OPENVR
        inline mat4(const vr::HmdMatrix44_t &a) :
            v00(a.m[0][0]),
            v10(a.m[1][0]),
            v20(a.m[2][0]),
            v30(a.m[3][0]),
            v01(a.m[0][1]),
            v11(a.m[1][1]),
            v21(a.m[2][1]),
            v31(a.m[3][1]),
            v02(a.m[0][2]),
            v12(a.m[1][2]),
            v22(a.m[2][2]),
            v32(a.m[3][2]),
            v03(a.m[0][3]),
            v13(a.m[1][3]),
            v23(a.m[2][3]),
            v33(a.m[3][3])
        {};

        inline mat4(const vr::HmdMatrix34_t &a) :
            v00(a.m[0][0]),
            v10(a.m[1][0]),
            v20(a.m[2][0]),
            v30(0.0f),
            v01(a.m[0][1]),
            v11(a.m[1][1]),
            v21(a.m[2][1]),
            v31(0.0f),
            v02(a.m[0][2]),
            v12(a.m[1][2]),
            v22(a.m[2][2]),
            v32(0.0f),
            v03(a.m[0][3]),
            v13(a.m[1][3]),
            v23(a.m[2][3]),
            v33(1.0f)
        {};
#endif

        inline ~mat4() {};
        
        inline mat4 & operator = (const mat4 &b)
        {
            v00 = b.v00;
            v10 = b.v10;
            v20 = b.v20;
            v30 = b.v30;
            v01 = b.v01;
            v11 = b.v11;
            v21 = b.v21;
            v31 = b.v31;
            v02 = b.v02;
            v12 = b.v12;
            v22 = b.v22;
            v32 = b.v32;
            v03 = b.v03;
            v13 = b.v13;
            v23 = b.v23;
            v33 = b.v33;
            return *this;
        };


        inline mat4 & operator *= (const mat4 &b)
        {
            const mat4 c(
                v00*b.v00 + v01*b.v10 + v02*b.v20 + v03*b.v30,
                v10*b.v00 + v11*b.v10 + v12*b.v20 + v13*b.v30,
                v20*b.v00 + v21*b.v10 + v22*b.v20 + v23*b.v30,
                v30*b.v00 + v31*b.v10 + v32*b.v20 + v33*b.v30,
                
                v00*b.v01 + v01*b.v11 + v02*b.v21 + v03*b.v31,
                v10*b.v01 + v11*b.v11 + v12*b.v21 + v13*b.v31,
                v20*b.v01 + v21*b.v11 + v22*b.v21 + v23*b.v31,
                v30*b.v01 + v31*b.v11 + v32*b.v21 + v33*b.v31,
                
                v00*b.v02 + v01*b.v12 + v02*b.v22 + v03*b.v32,
                v10*b.v02 + v11*b.v12 + v12*b.v22 + v13*b.v32,
                v20*b.v02 + v21*b.v12 + v22*b.v22 + v23*b.v32,
                v30*b.v02 + v31*b.v12 + v32*b.v22 + v33*b.v32,
                
                v00*b.v03 + v01*b.v13 + v02*b.v23 + v03*b.v33,
                v10*b.v03 + v11*b.v13 + v12*b.v23 + v13*b.v33,
                v20*b.v03 + v21*b.v13 + v22*b.v23 + v23*b.v33,
                v30*b.v03 + v31*b.v13 + v32*b.v23 + v33*b.v33);
            *this = c;
            return *this;
        };

        inline mat4 & operator *= (const float &b)
        {
            v00 *= b;
            v10 *= b;
            v20 *= b;
            v30 *= b;
            v01 *= b;
            v11 *= b;
            v21 *= b;
            v31 *= b;
            v02 *= b;
            v12 *= b;
            v22 *= b;
            v32 *= b;
            v03 *= b;
            v13 *= b;
            v23 *= b;
            v33 *= b;
            return *this;
        };

        inline vec3 operator * (const vec3 &a) const
        {
            return vec3(v00*a.x + v01*a.y + v02*a.z + v03,
                    v10*a.x + v11*a.y + v12*a.z + v13,
                    v20*a.x + v21*a.y + v22*a.z + v23);
        };

        inline void toOpenGL(float * const v) const
        {
            v[ 0] = v00;
            v[ 1] = v10;
            v[ 2] = v20;
            v[ 3] = v30;
            v[ 4] = v01;
            v[ 5] = v11;
            v[ 6] = v21;
            v[ 7] = v31;
            v[ 8] = v02;
            v[ 9] = v12;
            v[10] = v22;
            v[11] = v32;
            v[12] = v03;
            v[13] = v13;
            v[14] = v23;
            v[15] = v33;
        };
        
        inline mat4 transposed() const
        {
            return mat4(v00, v01, v02, v03,
                        v10, v11, v12, v13,
                        v20, v21, v22, v23,
                        v30, v31, v32, v33);
        };
        
        inline mat4 inverted() const
        {
            return mat4(v00, v01, v02, 0.0f,
                v10, v11, v12, 0.0f,
                v20, v21, v22, 0.0f,
                -(v00*v03 + v10*v13 + v20*v23),
                -(v01*v03 + v11*v13 + v21*v23),
                -(v02*v03 + v12*v13 + v22*v23),
                1.0f);
        };

        inline mat4 &setTranslation(const vec3 &a)
        {
            v03 = a.x;
            v13 = a.y;
            v23 = a.z;
            
            return *this;
        };
        
        inline vec3 getTranslation() const
        {
            return vec3(v03, v13, v23);
        };
        
        inline vec4 getRotation() const
        {
            //From http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm.
            const float t = v00 + v11 + v22;
            vec4 q;
            
            if (t > 0.0f)
            {
                const float s = 0.5f/sqrtf(1.0f + t);
                
                q = vec4((v21 - v12)*s, (v02 - v20)*s, (v10 - v01)*s, 0.25f/s);
            }
            else
            {
                if (v00 > v11 && v00 > v22)
                {
                    const float s = 0.5f/sqrtf(1.0f + v00 - v11 - v22);
                    
                    q = vec4(0.25f/s, (v01 + v10)*s, (v02 + v20)*s, (v21 - v12)*s);
                }
                else if (v11 > v22)
                {
                    const float s = 0.5f/sqrtf(1.0f + v11 - v00 - v22);
                    
                    q = vec4((v01 + v10)*s, 0.25f/s, (v12 + v21)*s, (v02 - v20)*s);
                }
                else
                {
                    const float s = 0.5f/sqrtf(1.0f + v22 - v00 - v11);
                    
                    q = vec4((v02 + v20)*s, (v12 + v21)*s, 0.25f/s, (v10 - v01)*s);
                }
            }
            
            return normalize(q);
        };
        
        static mat4 identityMatrix()
        {
            return mat4(1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0);
        };
        
        static mat4 translationMatrix(const vec3 &a)
        {
            return mat4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        a.x, a.y, a.z, 1.0f);
        };
        
        static mat4 scaleMatrix(const vec3 &a)
        {
            return mat4(a.x, 0.0, 0.0, 0.0,
                        0.0, a.y, 0.0, 0.0,
                        0.0, 0.0, a.z, 0.0,
                        0.0, 0.0, 0.0, 1.0);
        };
        
        static mat4 rotationMatrix(const vec4 &a)
        {
            return mat4(
                1.0f - 2.0f*(a.y*a.y + a.z*a.z),
                2.0f*(a.x*a.y - a.w*a.z),
                2.0f*(a.x*a.z + a.w*a.y),
                0.0f,
                2.0f*(a.x*a.y + a.w*a.z),
                1.0f - 2.0f*(a.x*a.x + a.z*a.z),
                2.0f*(a.y*a.z - a.w*a.x),
                0.0f,
                2.0f*(a.x*a.z - a.w*a.y),
                2.0f*(a.y*a.z + a.w*a.x),
                1.0f - 2.0f*(a.x*a.x + a.y*a.y),
                0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
        };
        
        static mat4 frustumMatrix(const vec3 &a, const vec3 &b)
        {
            const vec3 c = vec3(1.0f/(b.x - a.x), 1.0f/(b.y - a.y), 1.0f/(b.z - a.z));
            
            return mat4(2.0f*a.z*c.x,
                    0.0f,
                    0.0f,
                    0.0f,
                    
                    0.0f,
                    2.0f*a.z*c.y,
                    0.0f,
                    0.0f,
                    
                    (a.x + b.x)*c.x,
                    (a.y + b.y)*c.y,
                    -(a.z + b.z)*c.z,
                    -1.0,
                    
                    0.0f,
                    0.0f,
                    -2.0f*a.z*b.z*c.z,
                    0.0f);
        };
        
        float v00, v10, v20, v30;
        float v01, v11, v21, v31;
        float v02, v12, v22, v32;
        float v03, v13, v23, v33;
        
        friend std::ostream & operator << (std::ostream &Out, const mat4 &a)
        {
            Out << a.v00 << ", " << a.v01 << ", " << a.v02 << ", " << a.v03 << std::endl
                << a.v10 << ", " << a.v11 << ", " << a.v12 << ", " << a.v13 << std::endl
                << a.v20 << ", " << a.v21 << ", " << a.v22 << ", " << a.v23 << std::endl
                << a.v30 << ", " << a.v31 << ", " << a.v32 << ", " << a.v33;
            return Out;
        };
};

inline mat4 & operator * (mat4 a, const mat4 &b) {return a *= b;}
inline mat4 & operator * (const float &b, mat4 a) {return a *= b;}

vec4 quatmul(const vec4 &, const vec4 &);
vec4 quatconj(const vec4 &);
vec4 quatrot(const float &, const vec3 &);

vec2 randomVec2(const float & = 1.0f);
vec3 randomVec3(const float & = 1.0f);
vec4 randomVec4(const float & = 1.0f);

}

