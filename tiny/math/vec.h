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
#include <tuple>
#include <array>
#include <cmath>

#ifdef ENABLE_OPENVR
#include <openvr.h>
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


#define NRM_EPS 1.0e-8f
#define EPS 1.0e-6f

namespace tiny
{

//Why does C++ not offer a mod()-operation that is >= 0?
//From https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c.
template <typename t> inline t modnonneg(const t &a, const t &b) noexcept {return ((a % b) + b) % b;}
template <typename t> inline t t_abs(const t &a) noexcept {return (a < t(0) ? -a : a);}
template <typename t> inline t clamp(const t &a, const t &b, const t &c) noexcept {return (a < b ? b : (a > c ? c : a));}
template <typename t> inline t step(const t &a, const t &b) noexcept {return (b < a ? t(0) : t(1));}
template <typename t> inline t fract(const t &a) noexcept {return a - std::floor(a);}

template <typename t> class typed2vector
{
    public:
        inline typed2vector() noexcept : x(0), y(0) {};
        explicit inline typed2vector(const t &a) noexcept : x(a), y(a) {};
        inline typed2vector(const t &_x, const t &_y) noexcept : x(_x), y(_y) {};
        inline typed2vector(const typed2vector<t> &a) noexcept : x(a.x), y(a.y) {};
        inline typed2vector(typed2vector<t> &&a) noexcept : x(a.x), y(a.y) {};
        inline ~typed2vector() noexcept {};
        
        inline typed2vector & operator = (const typed2vector<t> &a) noexcept {x = a.x; y = a.y; return *this;};
        inline typed2vector & operator = (typed2vector<t> &&a) noexcept { x = a.x; y = a.y; return *this; };
        inline bool operator == (const typed2vector<t> &a) const noexcept {return (a.x == x && a.y == y);}
        inline bool operator != (const typed2vector<t> &a) const noexcept {return (a.x != x || a.y != y);}
        inline typed2vector & operator += (const typed2vector<t> &a) noexcept {x += a.x; y += a.y; return *this;};
        inline typed2vector & operator += (typed2vector<t> &&a) noexcept {x += a.x; y += a.y; return *this;};
        inline typed2vector & operator += (const t &a) noexcept {x += a; y += a; return *this;};
        inline typed2vector & operator -= (const typed2vector<t> &a) noexcept {x -= a.x; y -= a.y; return *this;};
        inline typed2vector & operator -= (const t &a) noexcept {x -= a; y -= a; return *this;};
        inline typed2vector & operator *= (const typed2vector<t> &a) noexcept {x *= a.x; y *= a.y; return *this;};
        inline typed2vector & operator /= (const typed2vector<t> &a) noexcept {x /= a.x; y /= a.y; return *this;};
        inline typed2vector & operator *= (const t &a) noexcept {x *= a; y *= a; return *this;};
        inline typed2vector & operator /= (const t &a) noexcept {x /= a; y /= a; return *this;};
        inline typed2vector operator - () const noexcept {return typed2vector<t>(-x, -y);};
        
        inline friend typed2vector<t> operator + (typed2vector<t> a, const typed2vector<t> &b) noexcept {return a += b;}
        inline friend typed2vector<t> operator + (typed2vector<t> a, const t &b) noexcept {return a += b;}
        inline friend typed2vector<t> operator + (const t &b, typed2vector<t> a) noexcept {return a += b;}
        inline friend typed2vector<t> operator - (typed2vector<t> a, const typed2vector<t> &b) noexcept {return a -= b;}
        inline friend typed2vector<t> operator - (typed2vector<t> a, const t &b) noexcept {return a -= b;}
        inline friend typed2vector<t> operator - (const t &b, typed2vector<t> a) noexcept {return typed2vector<t>(b - a.x, b - a.y);}
        inline friend typed2vector<t> operator * (typed2vector<t> a, const typed2vector<t> &b) noexcept {return a *= b;}
        inline friend typed2vector<t> operator * (typed2vector<t> a, const t &b) noexcept {return a *= b;}
        inline friend typed2vector<t> operator * (const t &b, typed2vector<t> a) noexcept {return a *= b;}
        inline friend typed2vector<t> operator / (typed2vector<t> a, const typed2vector<t> &b) noexcept {return a /= b;}
        inline friend typed2vector<t> operator / (typed2vector<t> a, const t &b) noexcept {return a /= b;}
        inline friend typed2vector<t> operator / (const t &b, typed2vector<t> a) noexcept {return typed2vector<t>(b/a.x, b/a.y);}

        inline typed2vector<t> xy() const noexcept {return typed2vector<t>(x, y);};
        inline typed2vector<t> yx() const noexcept {return typed2vector<t>(y, x);};
        
        inline friend typed2vector<t> min(const typed2vector<t> &a, const typed2vector<t> &b) noexcept {return typed2vector<t>(std::min(a.x, b.x), std::min(a.y, b.y));}
        inline friend typed2vector<t> max(const typed2vector<t> &a, const typed2vector<t> &b) noexcept {return typed2vector<t>(std::max(a.x, b.x), std::max(a.y, b.y));}
        inline friend typed2vector<t> abs(const typed2vector<t> &a) noexcept {return typed2vector<t>(t_abs(a.x), t_abs(a.y));}
        inline friend typed2vector<t> clamp(const typed2vector<t> &a, const typed2vector<t> &b, const typed2vector<t> &c) noexcept {return typed2vector<t>(clamp(a.x, b.x, c.x), clamp(a.y, b.y, c.y));}
        inline friend typed2vector<t> step(const typed2vector<t> &a, const typed2vector<t> &b) noexcept {return typed2vector<t>(step(a.x, b.x), step(a.y, b.y));}
        inline friend typed2vector<t> floor(const typed2vector<t> &a) noexcept {return typed2vector<t>(std::floor(a.x), std::floor(a.y));}
        inline friend typed2vector<t> ceil(const typed2vector<t> &a) noexcept {return typed2vector<t>(std::ceil(a.x), std::ceil(a.y));}
        inline friend typed2vector<t> fract(const typed2vector<t> &a) noexcept {return typed2vector<t>(fract(a.x), fract(a.y));}
        inline friend t dot(const typed2vector<t> &a, const typed2vector<t> &b) noexcept {return a.x*b.x + a.y*b.y;}
        inline friend t length(const typed2vector<t> &a) noexcept {return std::sqrt(dot(a, a));}
        inline friend t length2(const typed2vector<t> &a) noexcept {return dot(a, a);}
        inline friend typed2vector<t> normalize(const typed2vector<t> &a) noexcept {t l = 1.0f/std::max(length(a), NRM_EPS); return typed2vector<t>(a.x*l, a.y*l);}
        inline friend t minComponent(const typed2vector<t> &a) noexcept {return (a.x <= a.y ? a.x : a.y);}
        inline friend t maxComponent(const typed2vector<t> &a) noexcept {return (a.x >= a.y ? a.x : a.y);}

        friend std::ostream & operator << (std::ostream &Out, const typed2vector<t> &a) {Out << "(" << a.x << ", " << a.y << ")"; return Out;};
        
        t x, y;
};

template <typename t> class typed3vector
{
    public:
        inline typed3vector() noexcept : x(0), y(0), z(0) {};
        explicit inline typed3vector(const t &a) noexcept : x(a), y(a), z(a) {};
        inline typed3vector(const t &_x, const t &_y, const t &_z) noexcept : x(_x), y(_y), z(_z) {};
        explicit inline typed3vector(const typed2vector<t> &a, const t &_z) noexcept : x(a.x), y(a.y), z(_z) {};
        inline typed3vector(const typed3vector<t> &a) noexcept : x(a.x), y(a.y), z(a.z) {};
        inline typed3vector(typed3vector<t> &&a) noexcept : x(a.x), y(a.y), z(a.z) {};
        inline ~typed3vector() noexcept {};

        inline typed3vector & operator = (const typed3vector<t> &a) noexcept {x = a.x; y = a.y; z = a.z; return *this;};
        inline typed3vector & operator = (typed3vector<t> &&a) noexcept {x = a.x; y = a.y; z = a.z; return *this;};
        inline bool operator == (const typed3vector<t> &a) const noexcept {return (a.x == x && a.y == y && a.z == z);}
        inline bool operator != (const typed3vector<t> &a) const noexcept {return (a.x != x || a.y != y || a.z != z);}
        inline typed3vector & operator += (const typed3vector<t> &a) noexcept {x += a.x; y += a.y; z += a.z; return *this;};
        inline typed3vector & operator += (const t &a) noexcept {x += a; y += a; z += a; return *this;};
        inline typed3vector & operator -= (const typed3vector<t> &a) noexcept {x -= a.x; y -= a.y; z -= a.z; return *this;};
        inline typed3vector & operator -= (const t &a) noexcept {x -= a; y -= a; z -= a; return *this;};
        inline typed3vector & operator *= (const typed3vector<t> &a) noexcept {x *= a.x; y *= a.y; z *= a.z; return *this;};
        inline typed3vector & operator /= (const typed3vector<t> &a) noexcept {x /= a.x; y /= a.y; z /= a.z; return *this;};
        inline typed3vector & operator *= (const t &a) noexcept {x *= a; y *= a; z *= a; return *this;};
        inline typed3vector & operator /= (const t &a) noexcept {x /= a; y /= a; z /= a; return *this;};
        inline typed3vector operator - () const noexcept {return typed3vector<t>(-x, -y, -z);};
        
        inline friend typed3vector<t> operator + (typed3vector<t> a, const typed3vector<t> &b) noexcept {return a += b;}
        inline friend typed3vector<t> operator + (typed3vector<t> a, const t &b) noexcept {return a += b;}
        inline friend typed3vector<t> operator + (const t &b, typed3vector<t> a) noexcept {return a += b;}
        inline friend typed3vector<t> operator - (typed3vector<t> a, const typed3vector<t> &b) noexcept {return a -= b;}
        inline friend typed3vector<t> operator - (typed3vector<t> a, const t &b) noexcept {return a -= b;}
        inline friend typed3vector<t> operator - (const t &b, typed3vector<t> a) noexcept {return typed3vector<t>(b - a.x, b - a.y, b - a.z);}
        inline friend typed3vector<t> operator * (typed3vector<t> a, const typed3vector<t> &b) noexcept {return a *= b;}
        inline friend typed3vector<t> operator * (typed3vector<t> a, const t &b) noexcept {return a *= b;}
        inline friend typed3vector<t> operator * (const t &b, typed3vector<t> a) noexcept {return a *= b;}
        inline friend typed3vector<t> operator / (typed3vector<t> a, const typed3vector<t> &b) noexcept {return a /= b;}
        inline friend typed3vector<t> operator / (typed3vector<t> a, const t &b) noexcept {return a /= b;}
        inline friend typed3vector<t> operator / (const t &b, typed3vector<t> a) noexcept {return typed3vector<t>(b/a.x, b/a.y, b/a.z);}

        inline typed2vector<t> xy() const noexcept {return typed2vector<t>(x, y);};
        inline typed2vector<t> xz() const noexcept {return typed2vector<t>(x, z);};
        inline typed2vector<t> yz() const noexcept {return typed2vector<t>(y, z);};
        
        inline typed3vector<t> xyz() const noexcept {return typed3vector<t>(x, y, z);};
        inline typed3vector<t> xzy() const noexcept {return typed3vector<t>(x, z, y);};
        inline typed3vector<t> yxz() const noexcept {return typed3vector<t>(y, x, z);};
        inline typed3vector<t> yzx() const noexcept {return typed3vector<t>(y, z, x);};
        inline typed3vector<t> zxy() const noexcept {return typed3vector<t>(z, x, y);};
        inline typed3vector<t> zyx() const noexcept {return typed3vector<t>(z, y, x);};
        
        inline friend typed3vector<t> min(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return typed3vector<t>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z));}
        inline friend typed3vector<t> max(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return typed3vector<t>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z));}
        inline friend typed3vector<t> abs(const typed3vector<t> &a) noexcept {return typed3vector<t>(t_abs(a.x), t_abs(a.y), t_abs(a.z));}
        inline friend typed3vector<t> clamp(const typed3vector<t> &a, const typed3vector<t> &b, const typed3vector<t> &c) noexcept {return typed3vector<t>(clamp(a.x, b.x, c.x), clamp(a.y, b.y, c.y), clamp(a.z, b.z, c.z));}
        inline friend typed3vector<t> step(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return typed3vector<t>(step(a.x, b.x), step(a.y, b.y), step(a.z, b.z));}
        inline friend typed3vector<t> floor(const typed3vector<t> &a) noexcept {return typed3vector<t>(std::floor(a.x), std::floor(a.y), std::floor(a.z));}
        inline friend typed3vector<t> ceil(const typed3vector<t> &a) noexcept {return typed3vector<t>(std::ceil(a.x), std::ceil(a.y), std::ceil(a.z));}
        inline friend typed3vector<t> fract(const typed3vector<t> &a) noexcept {return typed3vector<t>(fract(a.x), fract(a.y), fract(a.z));}
        inline friend t dot(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return a.x*b.x + a.y*b.y + a.z*b.z;}
        inline friend typed3vector<t> cross(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return typed3vector<t>(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);}
        inline friend t length(const typed3vector<t> &a) noexcept {return std::sqrt(dot(a, a));}
        inline friend t length2(const typed3vector<t> &a) noexcept {return dot(a, a);}
        inline friend typed3vector<t> normalize(const typed3vector<t> &a) noexcept {t l = 1.0f/std::max(length(a), NRM_EPS); return typed3vector<t>(a.x*l, a.y*l, a.z*l);}
        inline friend t minComponent(const typed3vector<t> &a) noexcept {return (a.x <= a.y ? (a.x <= a.z ? a.x : a.z) : (a.y <= a.z ? a.y : a.z));}
        inline friend t maxComponent(const typed3vector<t> &a) noexcept {return (a.x >= a.y ? (a.x >= a.z ? a.x : a.z) : (a.y >= a.z ? a.y : a.z));}

        friend std::ostream & operator << (std::ostream &Out, const typed3vector<t> &a) {Out << "(" << a.x << ", " << a.y << ", " << a.z << ")"; return Out;};
        
        t x, y, z;
};

template <typename t> class typed4vector
{
    public:
        inline typed4vector() noexcept : x(0), y(0), z(0), w(0) {};
        explicit inline typed4vector(const t &a) noexcept : x(a), y(a), z(a), w(a) {};
        inline typed4vector(const t &_x, const t &_y, const t &_z, const t &_w) noexcept : x(_x), y(_y), z(_z), w(_w) {};
        inline typed4vector(const typed2vector<t> &a, const t &_z, const t &_w) noexcept : x(a.x), y(a.y), z(_z), w(_w) {};
        inline typed4vector(const typed3vector<t> &a, const t &_w) noexcept : x(a.x), y(a.y), z(a.z), w(_w) {};
        inline typed4vector(const typed4vector<t> &a) noexcept : x(a.x), y(a.y), z(a.z), w(a.w) {};
        inline typed4vector(typed4vector<t> &&a) noexcept : x(a.x), y(a.y), z(a.z), w(a.w) {};
        inline ~typed4vector() noexcept {};
        
        inline typed4vector & operator = (const typed4vector<t> &a) noexcept {x = a.x; y = a.y; z = a.z; w = a.w; return *this;};
        inline typed4vector & operator = (typed4vector<t> &&a) noexcept {x = a.x; y = a.y; z = a.z; w = a.w; return *this;};
        inline bool operator == (const typed4vector<t> &a) const noexcept {return (a.x == x && a.y == y && a.z == z && a.w == w);}
        inline bool operator != (const typed4vector<t> &a) const noexcept {return (a.x != x || a.y != y || a.z != z || a.w != w);}
        inline typed4vector & operator += (const typed4vector<t> &a) noexcept {x += a.x; y += a.y; z += a.z; w += a.w; return *this;};
        inline typed4vector & operator += (const t &a) noexcept {x += a; y += a; z += a; w += a; return *this;};
        inline typed4vector & operator -= (const typed4vector<t> &a) noexcept {x -= a.x; y -= a.y; z -= a.z; w -= a.w; return *this;};
        inline typed4vector & operator -= (const t &a) noexcept {x -= a; y -= a; z -= a; w -= a; return *this;};
        inline typed4vector & operator *= (const typed4vector<t> &a) noexcept {x *= a.x; y *= a.y; z *= a.z; w *= a.w; return *this;};
        inline typed4vector & operator /= (const typed4vector<t> &a) noexcept {x /= a.x; y /= a.y; z /= a.z; w /= a.w; return *this;};
        inline typed4vector & operator *= (const t &a) noexcept {x *= a; y *= a; z *= a; w *= a; return *this;};
        inline typed4vector & operator /= (const t &a) noexcept {x /= a; y /= a; z /= a; w /= a; return *this;};
        inline typed4vector operator - () const noexcept {return typed4vector<t>(-x, -y, -z, -w);};
        
        inline friend typed4vector<t> operator + (typed4vector<t> a, const typed4vector<t> &b) noexcept {return a += b;}
        inline friend typed4vector<t> operator + (typed4vector<t> a, const t &b) noexcept {return a += b;}
        inline friend typed4vector<t> operator + (const t &b, typed4vector<t> a) noexcept {return a += b;}
        inline friend typed4vector<t> operator - (typed4vector<t> a, const typed4vector<t> &b) noexcept {return a -= b;}
        inline friend typed4vector<t> operator - (typed4vector<t> a, const t &b) noexcept {return a -= b;}
        inline friend typed4vector<t> operator - (const t &b, typed4vector<t> a) noexcept {return typed4vector<t>(b - a.x, b - a.y, b - a.z, b - a.w);}
        inline friend typed4vector<t> operator * (typed4vector<t> a, const typed4vector<t> &b) noexcept {return a *= b;}
        inline friend typed4vector<t> operator * (typed4vector<t> a, const t &b) noexcept {return a *= b;}
        inline friend typed4vector<t> operator * (const t &b, typed4vector<t> a) noexcept {return a *= b;}
        inline friend typed4vector<t> operator / (typed4vector<t> a, const typed4vector<t> &b) noexcept {return a /= b;}
        inline friend typed4vector<t> operator / (typed4vector<t> a, const t &b) noexcept {return a /= b;}
        inline friend typed4vector<t> operator / (const t &b, typed4vector<t> a) noexcept {return typed4vector<t>(b/a.x, b/a.y, b/a.z, b/a.w);}
        
        inline typed2vector<t> xy() const noexcept {return typed2vector<t>(x, y);};
        inline typed3vector<t> xyz() const noexcept {return typed3vector<t>(x, y, z);};
        
        inline friend typed4vector<t> min(const typed4vector<t> &a, const typed4vector<t> &b) noexcept {return typed4vector<t>(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z), std::min(a.w, b.w));}
        inline friend typed4vector<t> max(const typed4vector<t> &a, const typed4vector<t> &b) noexcept {return typed4vector<t>(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z), std::max(a.w, b.w));}
        inline friend typed4vector<t> abs(const typed4vector<t> &a) noexcept {return typed4vector<t>(t_abs(a.x), t_abs(a.y), t_abs(a.z), t_abs(a.w));}
        inline friend typed4vector<t> clamp(const typed4vector<t> &a, const typed4vector<t> &b, const typed4vector<t> &c) noexcept {return typed4vector<t>(clamp(a.x, b.x, c.x), clamp(a.y, b.y, c.y), clamp(a.z, b.z, c.z), clamp(a.w, b.w, c.w));}
        inline friend typed4vector<t> step(const typed4vector<t> &a, const typed4vector<t> &b) noexcept {return typed4vector<t>(step(a.x, b.x), step(a.y, b.y), step(a.z, b.z), step(a.w, b.w));}
        inline friend typed4vector<t> floor(const typed4vector<t> &a) noexcept {return typed4vector<t>(std::floor(a.x), std::floor(a.y), std::floor(a.z), std::floor(a.w));}
        inline friend typed4vector<t> ceil(const typed4vector<t> &a) noexcept {return typed4vector<t>(std::ceil(a.x), std::ceil(a.y), std::ceil(a.z), std::ceil(a.w));}
        inline friend typed4vector<t> fract(const typed4vector<t> &a) noexcept {return typed4vector<t>(fract(a.x), fract(a.y), fract(a.z), fract(a.w));}
        inline friend t dot(const typed4vector<t> &a, const typed4vector<t> &b) noexcept {return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;}
        inline friend t length(const typed4vector<t> &a) noexcept {return std::sqrt(dot(a, a));}
        inline friend t length2(const typed4vector<t> &a) noexcept {return dot(a, a);}
        inline friend typed4vector<t> normalize(const typed4vector<t> &a) noexcept {t l = 1.0f/std::max(length(a), NRM_EPS); return typed4vector<t>(a.x*l, a.y*l, a.z*l, a.w*l);}

        friend std::ostream & operator << (std::ostream &Out, const typed4vector<t> &a) {Out << "(" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << ")"; return Out;};
        
        t x, y, z, w;
};

typedef typed2vector<int> ivec2;
typedef typed2vector<float> vec2;
typedef typed3vector<int> ivec3;
typedef typed3vector<float> vec3;
typedef typed4vector<int> ivec4;
typedef typed4vector<float> vec4;

inline vec2 to_float(const ivec2 &a) noexcept {return vec2(static_cast<float>(a.x), static_cast<float>(a.y));}
inline vec3 to_float(const ivec3 &a) noexcept {return vec3(static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z));}
inline vec4 to_float(const ivec4 &a) noexcept {return vec4(static_cast<float>(a.x), static_cast<float>(a.y), static_cast<float>(a.z), static_cast<float>(a.w));}
inline ivec2 to_int(const vec2 &a) noexcept {return ivec2(static_cast<int>(a.x), static_cast<int>(a.y));}
inline ivec3 to_int(const vec3 &a) noexcept {return ivec3(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(a.z));}
inline ivec4 to_int(const vec4 &a) noexcept {return ivec4(static_cast<int>(a.x), static_cast<int>(a.y), static_cast<int>(a.z), static_cast<int>(a.w));}

template <typename t> inline ivec2 lessThanEqual(const typed2vector<t> &a, const typed2vector<t> &b) noexcept {return ivec2(a.x <= b.x ? 1 : 0, a.y <= b.y ? 1 : 0);}
template <typename t> inline ivec3 lessThanEqual(const typed3vector<t> &a, const typed3vector<t> &b) noexcept {return ivec3(a.x <= b.x ? 1 : 0, a.y <= b.y ? 1 : 0, a.z <= b.z ? 1 : 0);}
template <typename t> inline ivec4 lessThanEqual(const typed4vector<t> &a, const typed4vector<t> &b) noexcept {return ivec4(a.x <= b.x ? 1 : 0, a.y <= b.y ? 1 : 0, a.z <= b.z ? 1 : 0, a.w <= b.w ? 1 : 0);}

//Integer specific bit operations.
inline ivec2 operator % (const ivec2 &a, const ivec2 &b) noexcept {return ivec2(a.x % b.x, a.y % b.y);}
inline ivec2 operator % (const ivec2 &a, const int &b) noexcept {return ivec2(a.x % b, a.y % b);}
inline ivec2 operator & (const ivec2 &a, const ivec2 &b) noexcept {return ivec2(a.x & b.x, a.y & b.y);}
inline ivec2 operator & (const ivec2 &a, const int &b) noexcept {return ivec2(a.x & b, a.y & b);}
inline ivec2 operator | (const ivec2 &a, const ivec2 &b) noexcept {return ivec2(a.x | b.x, a.y | b.y);}
inline ivec2 operator | (const ivec2 &a, const int &b) noexcept {return ivec2(a.x | b, a.y | b);}
inline ivec2 operator >> (const ivec2 &a, const ivec2 &b) noexcept {return ivec2(a.x >> b.x, a.y >> b.y);}
inline ivec2 operator >> (const ivec2 &a, const int &b) noexcept {return ivec2(a.x >> b, a.y >> b);}
inline ivec2 operator << (const ivec2 &a, const ivec2 &b) noexcept {return ivec2(a.x << b.x, a.y << b.y);}
inline ivec2 operator << (const ivec2 &a, const int &b) noexcept {return ivec2(a.x << b, a.y << b);}
inline bool operator < (const ivec2 &a, const ivec2 &b) noexcept {return (a.x == b.x ? a.y < b.y : a.x < b.x);}

//Quaternion specific operations.
inline vec4 quatmul(const vec4 &a, const vec4 &b) noexcept
{
    return vec4(a.w*b.x + b.w*a.x + a.y*b.z - a.z*b.y,
        a.w*b.y + b.w*a.y + a.z*b.x - a.x*b.z,
        a.w*b.z + b.w*a.z + a.x*b.y - a.y*b.x,
        a.w*b.w - (a.x*b.x + a.y*b.y + a.z*b.z));
}

inline vec4 quatconj(const vec4 &a) noexcept
{
    return vec4(-a.x, -a.y, -a.z, a.w);
}

inline vec4 quatrot(const float &alpha, const vec3 &a) noexcept
{
    const float s = sin(0.5f*alpha);
    
    return vec4(s*a.x, s*a.y, s*a.z, cos(0.5f*alpha));
}

vec2 randomVec2(const float & = 1.0f);
vec3 randomVec3(const float & = 1.0f);
vec4 randomVec4(const float & = 1.0f);

class mat2
{
    public:
        mat2() = default;
        //FIXME: Causes uninitialized-variables errors (even for initialized data!).
        //mat2(const mat2 &) = default;
        inline mat2(const mat2 &a) noexcept :
            v00(a.v00), v10(a.v10),
            v01(a.v01), v11(a.v11)
        {};
        mat2(mat2 &&) = default;
        mat2 & operator = (const mat2 &) = default;
        ~mat2() = default;

        inline mat2(const float &a) noexcept :
            v00(a), v10(a),
            v01(a), v11(a)
        {};
        inline mat2(const float &_v00, const float &_v10,
                const float &_v01, const float &_v11) noexcept :
            v00(_v00), v10(_v10),
            v01(_v01), v11(_v11)
        {};
        
        explicit inline mat2(const vec2 &x, const vec2 &y) noexcept :
            v00(x.x), v10(x.y),
            v01(y.x), v11(y.y)
        {};
        
        inline mat2 & operator += (const mat2 &b) noexcept
        {
            v00 += b.v00;
            v10 += b.v10;
            v01 += b.v01;
            v11 += b.v11;
            return *this;
        };

        inline mat2 & operator -= (const mat2 &b) noexcept
        {
            v00 -= b.v00;
            v10 -= b.v10;
            v01 -= b.v01;
            v11 -= b.v11;
            return *this;
        };
        
        inline mat2 & operator *= (const mat2 &b) noexcept
        {
            const mat2 c(
                v00*b.v00 + v01*b.v10,
                v10*b.v00 + v11*b.v10,
                
                v00*b.v01 + v01*b.v11,
                v10*b.v01 + v11*b.v11);
            *this = c;
            return *this;
        };
        
        inline mat2 & operator *= (const float &b) noexcept
        {
            v00 *= b;
            v10 *= b;
            v01 *= b;
            v11 *= b;
            return *this;
        };

        inline mat2 & operator /= (const float &b) noexcept
        {
            v00 /= b;
            v10 /= b;
            v01 /= b;
            v11 /= b;
            return *this;
        };

        inline friend mat2 & operator + (mat2 a, const mat2 &b) noexcept {return a += b;}
        inline friend mat2 & operator - (mat2 a, const mat2 &b) noexcept {return a -= b;}
        inline friend mat2 & operator * (mat2 a, const mat2 &b) noexcept {return a *= b;}
        inline friend mat2 & operator * (const float &b, mat2 a) noexcept {return a *= b;}
        inline friend mat2 & operator * (mat2 a, const float &b) noexcept {return a *= b;}
        inline friend mat2 & operator / (mat2 a, const float &b) noexcept {return a /= b;}

        inline vec2 operator * (const vec2 &a) const noexcept
        {
            return vec2(v00*a.x + v01*a.y,
                    v10*a.x + v11*a.y);
        };

        inline mat2 transposed() const noexcept
        {
            return mat2(v00, v01,
                        v10, v11);
        };
        
        inline mat2 adjugated() const noexcept
        {
            return mat2( v11, -v10,
                        -v01,  v00);
        };
        
        inline mat2 inverted() const noexcept
        {
            const float t = v00*v11 - v10*v01;

            return mat2( v11/t, -v10/t,
                        -v01/t,  v00/t);
        };

        inline static mat2 identityMatrix() noexcept
        {
            return mat2(1.0, 0.0,
                        0.0, 1.0);
        };

        float v00, v10;
        float v01, v11;
        
        friend std::ostream & operator << (std::ostream &Out, const mat2 &a)
        {
            Out << a.v00 << ", " << a.v01 << std::endl
                << a.v10 << ", " << a.v11 << std::endl;
            return Out;
        };
};

class mat3
{
    public:
        mat3() = default;
        //FIXME: Causes uninitialized-variables errors (even for initialized data!).
        //mat3(const mat3 &) = default;
        inline mat3(const mat3 &a) noexcept :
            v00(a.v00), v10(a.v10), v20(a.v20),
            v01(a.v01), v11(a.v11), v21(a.v21),
            v02(a.v02), v12(a.v12), v22(a.v22)
        {};
        mat3(mat3 &&) = default;
        mat3 & operator = (const mat3 &) = default;
        ~mat3() = default;

        inline mat3(const float &a) noexcept :
            v00(a), v10(a), v20(a),
            v01(a), v11(a), v21(a),
            v02(a), v12(a), v22(a)
        {};
        inline mat3(const float &_v00, const float &_v10, const float &_v20,
                const float &_v01, const float &_v11, const float &_v21,
                const float &_v02, const float &_v12, const float &_v22) noexcept :
            v00(_v00), v10(_v10), v20(_v20),
            v01(_v01), v11(_v11), v21(_v21),
            v02(_v02), v12(_v12), v22(_v22)
        {};
        
        inline mat3(const vec3 &x, const vec3 &y, const vec3 &z) noexcept :
            v00(x.x), v10(x.y), v20(x.z),
            v01(y.x), v11(y.y), v21(y.z),
            v02(z.x), v12(z.y), v22(z.z)
        {};

        inline mat3 & operator += (const mat3 &b) noexcept
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

        inline mat3 & operator -= (const mat3 &b) noexcept
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

        inline mat3 & operator *= (const mat3 &b) noexcept
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

        inline mat3 & operator *= (const float &b) noexcept
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

        inline mat3 & operator /= (const float &b) noexcept
        {
            v00 /= b;
            v10 /= b;
            v20 /= b;
            v01 /= b;
            v11 /= b;
            v21 /= b;
            v02 /= b;
            v12 /= b;
            v22 /= b;
            return *this;
        };

        inline friend mat3 & operator + (mat3 a, const mat3 &b) noexcept {return a += b;}
        inline friend mat3 & operator - (mat3 a, const mat3 &b) noexcept {return a -= b;}
        inline friend mat3 & operator * (mat3 a, const mat3 &b) noexcept {return a *= b;}
        inline friend mat3 & operator * (const float &b, mat3 a) noexcept {return a *= b;}
        inline friend mat3 & operator * (mat3 a, const float &b) noexcept {return a *= b;}
        inline friend mat3 & operator / (mat3 a, const float &b) noexcept {return a /= b;}

        inline vec3 operator * (const vec3 &a) const noexcept
        {
            return vec3(v00*a.x + v01*a.y + v02*a.z,
                    v10*a.x + v11*a.y + v12*a.z,
                    v20*a.x + v21*a.y + v22*a.z);
        };

        inline void toOpenGL(float * const v) const noexcept
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
        
        inline mat3 transposed() const noexcept
        {
            return mat3(v00, v01, v02,
                        v10, v11, v12,
                        v20, v21, v22);
        };
        
        inline mat3 inverted() const noexcept
        {
            const float t = v00*(v11*v22 - v12*v21) - v01*(v10*v22 - v12*v20) + v02*(v10*v21 - v11*v20);

            return mat3(
                    (v11*v22 - v12*v21)/t, (v12*v20 - v10*v22)/t, (v10*v21 - v11*v20)/t,
                    (v02*v21 - v01*v22)/t, (v00*v22 - v02*v20)/t, (v01*v20 - v00*v21)/t,
                    (v01*v12 - v02*v11)/t, (v02*v10 - v00*v12)/t, (v00*v11 - v01*v10)/t);
        };

        std::tuple<vec3, mat3> eigenDecompositionSym() const;
        
        inline vec4 getRotation() const noexcept
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
        
        inline static mat3 identityMatrix() noexcept
        {
            return mat3(1.0, 0.0, 0.0,
                        0.0, 1.0, 0.0,
                        0.0, 0.0, 1.0);
        };

        inline static mat3 outerProductMatrix(const vec3 &a, const vec3 &b) noexcept
        {
            return mat3(a.x*b.x, a.y*b.x, a.z*b.x,
                        a.x*b.y, a.y*b.y, a.z*b.y,
                        a.x*b.z, a.y*b.z, a.z*b.z);
        };
        
        inline static mat3 scaleMatrix(const vec3 &a) noexcept
        {
            return mat3(a.x, 0.0, 0.0,
                        0.0, a.y, 0.0,
                        0.0, 0.0, a.z);
        };
        
        inline static mat3 rotationMatrix(const vec4 &a) noexcept
        {
            return mat3(
                1.0f - 2.0f*(a.y*a.y + a.z*a.z),
                2.0f*(a.x*a.y + a.w*a.z),
                2.0f*(a.x*a.z - a.w*a.y),
                2.0f*(a.x*a.y - a.w*a.z),
                1.0f - 2.0f*(a.x*a.x + a.z*a.z),
                2.0f*(a.y*a.z + a.w*a.x),
                2.0f*(a.x*a.z + a.w*a.y),
                2.0f*(a.y*a.z - a.w*a.x),
                1.0f - 2.0f*(a.x*a.x + a.y*a.y));
        };

        inline float getFrobeniusNorm() const noexcept
        {
            return std::sqrt(v00*v00 + v10*v10 + v20*v20 +
                             v01*v01 + v11*v11 + v21*v21 +
                             v02*v02 + v12*v12 + v22*v22);
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

class mat4
{
    public:
        mat4() = default;
        //FIXME: Causes uninitialized-variables errors (even for initialized data!).
        //mat4(const mat4 &) = default;
        inline mat4(const mat4 &a) noexcept :
            v00(a.v00), v10(a.v10), v20(a.v20), v30(a.v30),
            v01(a.v01), v11(a.v11), v21(a.v21), v31(a.v31),
            v02(a.v02), v12(a.v12), v22(a.v22), v32(a.v32),
            v03(a.v03), v13(a.v13), v23(a.v23), v33(a.v33)
        {};
        mat4(mat4 &&) = default;
        mat4 & operator = (const mat4 &) = default;
        ~mat4() = default;

        inline mat4(const float &a) noexcept :
            v00(a), v10(a), v20(a), v30(a),
            v01(a), v11(a), v21(a), v31(a),
            v02(a), v12(a), v22(a), v32(a),
            v03(a), v13(a), v23(a), v33(a)
        {};
        inline mat4(const float &_v00, const float &_v10, const float &_v20, const float &_v30,
                const float &_v01, const float &_v11, const float &_v21, const float &_v31,
                const float &_v02, const float &_v12, const float &_v22, const float &_v32,
                const float &_v03, const float &_v13, const float &_v23, const float &_v33) noexcept :
            v00(_v00), v10(_v10), v20(_v20), v30(_v30),
            v01(_v01), v11(_v11), v21(_v21), v31(_v31),
            v02(_v02), v12(_v12), v22(_v22), v32(_v32),
            v03(_v03), v13(_v13), v23(_v23), v33(_v33)
        {};

        explicit inline mat4(const vec3 &x, const vec3 &y, const vec3 &z, const vec3 &b = vec3(0.0f, 0.0f, 0.0f)) noexcept :
            v00(x.x), v10(x.y), v20(x.z), v30(0.0f),
            v01(y.x), v11(y.y), v21(y.z), v31(0.0f),
            v02(z.x), v12(z.y), v22(z.z), v32(0.0f),
            v03(b.x), v13(b.y), v23(b.z), v33(1.0f)
        {};
        
#ifdef ENABLE_OPENVR
        inline mat4(const vr::HmdMatrix44_t &a) noexcept :
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

        inline mat4(const vr::HmdMatrix34_t &a) noexcept :
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
        inline mat4 & operator += (const mat4 &b) noexcept
        {
            v00 += b.v00;
            v10 += b.v10;
            v20 += b.v20;
            v30 += b.v30;
            v01 += b.v01;
            v11 += b.v11;
            v21 += b.v21;
            v31 += b.v31;
            v02 += b.v02;
            v12 += b.v12;
            v22 += b.v22;
            v32 += b.v32;
            v03 += b.v03;
            v13 += b.v13;
            v23 += b.v23;
            v33 += b.v33;
            return *this;
        };

        inline mat4 & operator -= (const mat4 &b) noexcept
        {
            v00 -= b.v00;
            v10 -= b.v10;
            v20 -= b.v20;
            v30 -= b.v30;
            v01 -= b.v01;
            v11 -= b.v11;
            v21 -= b.v21;
            v31 -= b.v31;
            v02 -= b.v02;
            v12 -= b.v12;
            v22 -= b.v22;
            v32 -= b.v32;
            v03 -= b.v03;
            v13 -= b.v13;
            v23 -= b.v23;
            v33 -= b.v33;
            return *this;
        };

        inline mat4 & operator *= (const mat4 &b) noexcept
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

        inline mat4 & operator *= (const float &b) noexcept
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
        
        inline mat4 & operator /= (const float &b) noexcept
        {
            v00 /= b;
            v10 /= b;
            v20 /= b;
            v30 /= b;
            v01 /= b;
            v11 /= b;
            v21 /= b;
            v31 /= b;
            v02 /= b;
            v12 /= b;
            v22 /= b;
            v32 /= b;
            v03 /= b;
            v13 /= b;
            v23 /= b;
            v33 /= b;
            return *this;
        };
        
        inline friend mat4 & operator + (mat4 a, const mat4 &b) noexcept {return a += b;}
        inline friend mat4 & operator - (mat4 a, const mat4 &b) noexcept {return a -= b;}
        inline friend mat4 & operator * (mat4 a, const mat4 &b) noexcept {return a *= b;}
        inline friend mat4 & operator * (const float &b, mat4 a) noexcept {return a *= b;}
        inline friend mat4 & operator * (mat4 a, const float &b) noexcept {return a *= b;}
        inline friend mat4 & operator / (mat4 a, const float &b) noexcept {return a /= b;}

        inline vec3 operator * (const vec3 &a) const noexcept
        {
            return vec3(v00*a.x + v01*a.y + v02*a.z + v03,
                        v10*a.x + v11*a.y + v12*a.z + v13,
                        v20*a.x + v21*a.y + v22*a.z + v23);
        };

        inline vec4 operator * (const vec4 &a) const noexcept
        {
            return vec4(v00*a.x + v01*a.y + v02*a.z + v03*a.w,
                        v10*a.x + v11*a.y + v12*a.z + v13*a.w,
                        v20*a.x + v21*a.y + v22*a.z + v23*a.w,
                        v30*a.x + v31*a.y + v32*a.z + v33*a.w);
        };

        inline void toOpenGL(float * const v) const noexcept
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
        
        inline mat4 transposed() const noexcept
        {
            return mat4(v00, v01, v02, v03,
                        v10, v11, v12, v13,
                        v20, v21, v22, v23,
                        v30, v31, v32, v33);
        };
        
        inline mat4 inverted() const noexcept
        {
            return mat4(v00, v01, v02, 0.0f,
                v10, v11, v12, 0.0f,
                v20, v21, v22, 0.0f,
                -(v00*v03 + v10*v13 + v20*v23),
                -(v01*v03 + v11*v13 + v21*v23),
                -(v02*v03 + v12*v13 + v22*v23),
                1.0f);
        };
        
        inline mat4 invertedFull() const noexcept
        {
            //Less efficient version of https://lxjk.github.io/2017/09/03/Fast-4x4-Matrix-Inverse-with-SSE-SIMD-Explained.html.
            const mat2 A = mat2(v00, v10, v01, v11);
            const mat2 B = mat2(v02, v12, v03, v13);
            const mat2 C = mat2(v20, v30, v21, v31);
            const mat2 D = mat2(v22, v32, v23, v33);
            const float dA = v00*v11 - v01*v10;
            const float dB = v02*v13 - v03*v12;
            const float dC = v20*v31 - v21*v30;
            const float dD = v22*v33 - v23*v32;
            
            const mat2 X = (A*dD - B*(D.adjugated()*C)).adjugated();
            const mat2 Y = (C*dB - D*(B.adjugated()*A)).adjugated();
            const mat2 Z = (B*dC - A*(C.adjugated()*D)).adjugated();
            const mat2 W = (D*dA - C*(A.adjugated()*B)).adjugated();
            
            const float dM = dA*dD - dB*dC;
            
            return mat4(X.v00, X.v10, Z.v00, Z.v10,
                        X.v01, X.v11, Z.v01, Z.v11,
                        Y.v00, Y.v10, W.v00, W.v10,
                        Y.v01, Y.v11, W.v01, W.v11)/dM;
        };

        inline mat4 &setTranslation(const vec3 &a) noexcept
        {
            v03 = a.x;
            v13 = a.y;
            v23 = a.z;
            
            return *this;
        };
        
        inline vec3 getTranslation() const noexcept
        {
            return vec3(v03, v13, v23);
        };
        
        inline vec4 getRotation() const noexcept
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
        
        inline static mat4 identityMatrix() noexcept
        {
            return mat4(1.0, 0.0, 0.0, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0);
        };
        
        inline static mat4 translationMatrix(const vec3 &a) noexcept
        {
            return mat4(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        a.x, a.y, a.z, 1.0f);
        };
        
        inline static mat4 from3x3(const mat3 &a) noexcept
        {
            return mat4(a.v00, a.v10, a.v20, 0.0f,
                        a.v01, a.v11, a.v21, 0.0f,
                        a.v02, a.v12, a.v22, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
        };

        inline static mat4 scaleMatrix(const vec3 &a) noexcept
        {
            return mat4(a.x, 0.0, 0.0, 0.0,
                        0.0, a.y, 0.0, 0.0,
                        0.0, 0.0, a.z, 0.0,
                        0.0, 0.0, 0.0, 1.0);
        };
        
        inline static mat4 rotationMatrix(const vec4 &a) noexcept
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
        
        inline static mat4 rotationTranslationMatrix(const vec4 &a, const vec3 &b) noexcept
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
                b.x, b.y, b.z, 1.0f);
        };

        inline static mat4 frustumMatrix(const vec3 &a, const vec3 &b) noexcept
        {
            const vec3 c = 1.0f/(b - a);
            
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

}

