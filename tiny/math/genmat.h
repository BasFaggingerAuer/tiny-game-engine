/*
Copyright 2022, Bas Fagginger Auer.

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

#include <type_traits>
#include <array>
#include <algorithm>
#include <cassert>

#include <tiny/math/vec.h>

//(Slow, should use LAPACK/Eigen/..., but this code will only be used incidentally and it's fun to figure this out.)
namespace tiny
{

//Generic matrix class.
template<typename t, size_t m, size_t n>
class genmat
{
    public:
        std::array<t, m*n> v;
        typedef t value;

    public:
        genmat() = default;
        genmat(const genmat<t, m, n> &) = default;
        genmat(genmat<t, m, n> &&) = default;
        genmat<t, m, n> & operator = (const genmat<t, m, n> &) = default;
        genmat<t, m, n> & operator = (genmat<t, m, n> &&) = default;
        ~genmat() = default;

        //Specific constructors.
        template <typename ...Args>
        genmat(Args ...args) noexcept : v({args...}) {}

        explicit genmat(const t &a) noexcept {v.fill(a);}
        
        //Coefficients.
        inline const t & operator () (const size_t i, const size_t j) const noexcept {return v[m*j + i];};
        inline t & operator () (const size_t i, const size_t j) noexcept {return v[m*j + i];};

        //Coefficient-wise operations.
        inline friend genmat<t, m, n> clamp(const genmat<t, m, n> &a, const genmat<t, m, n> &b, const genmat<t, m, n> &c) noexcept
        {
            genmat<t, m, n> d;
            for (size_t i = 0; i < m*n; ++i) d.v[i] = clamp(a.v[i], b.v[i], c.v[i]);
            return d;
        }
        inline friend genmat<t, m, n> step(const genmat<t, m, n> &a, const genmat<t, m, n> &b) noexcept
        {
            genmat<t, m, n> c;
            for (size_t i = 0; i < m*n; ++i) c.v[i] = step(a.v[i], b.v[i]);
            return c;
        }
        inline friend genmat<t, m, n> min(const genmat<t, m, n> &a, const genmat<t, m, n> &b) noexcept
        {
            genmat<t, m, n> c;
            for (size_t i = 0; i < m*n; ++i) c.v[i] = std::min<t>(a.v[i], b.v[i]);
            return c;
        }
        inline friend genmat<t, m, n> max(const genmat<t, m, n> &a, const genmat<t, m, n> &b) noexcept
        {
            genmat<t, m, n> c;
            for (size_t i = 0; i < m*n; ++i) c.v[i] = std::max<t>(a.v[i], b.v[i]);
            return c;
        }
        inline friend genmat<t, m, n> abs(const genmat<t, m, n> &a) noexcept
        {
            genmat<t, m, n> b;
            for (size_t i = 0; i < m*n; ++i) b.v[i] = t_abs(a.v[i]);
            return b;
        }
        inline friend genmat<t, m, n> floor(const genmat<t, m, n> &a) noexcept
        {
            genmat<t, m, n> b;
            for (size_t i = 0; i < m*n; ++i) b.v[i] = std::floor(a.v[i]);
            return b;
        }
        inline friend genmat<t, m, n> ceil(const genmat<t, m, n> &a) noexcept
        {
            genmat<t, m, n> b;
            for (size_t i = 0; i < m*n; ++i) b.v[i] = std::ceil(a.v[i]);
            return b;
        }
        inline friend genmat<t, m, n> fract(const genmat<t, m, n> &a) noexcept
        {
            genmat<t, m, n> b;
            for (size_t i = 0; i < m*n; ++i) b.v[i] = fract(a.v[i]);
            return b;
        }

        //Equality/comparison.
        inline bool operator == (const genmat<t, m, n> &a) const noexcept
        {
            for (size_t i = 0; i < m*n; ++i) if (v[i] != a.v[i]) return false;
            return true;
        }

        inline bool operator < (const genmat<t, m, n> &a) const noexcept
        {
            for (size_t i = 0; i < m*n; ++i)
            {
                if (v[i] < a.v[i]) return true;
                if (v[i] > a.v[i]) return false;
            }

            return false;
        }
        
        //Addition.
        inline genmat<t, m, n> & operator += (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] += a.v[i];
            return *this;
        }

        inline genmat<t, m, n> & operator += (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] += a.v[i];
            return *this;
        }

        inline friend genmat<t, m, n> operator + (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a += b;}
        
        inline genmat<t, m, n> & operator += (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] += a;
            return *this;
        }

        inline friend genmat<t, m, n> operator + (genmat<t, m, n> a, const t &b) noexcept {return a += b;}

        //Subtraction.
        inline genmat<t, m, n> & operator -= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] -= a.v[i];
            return *this;
        }

        inline genmat<t, m, n> & operator -= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] -= a.v[i];
            return *this;
        }

        inline friend genmat<t, m, n> operator - (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a -= b;}
        
        inline genmat<t, m, n> & operator -= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] -= a;
            return *this;
        }

        inline friend genmat<t, m, n> operator - (genmat<t, m, n> a, const t &b) noexcept {return a -= b;}

        inline genmat<t, m, n> operator - () const noexcept
        {
            genmat<t, m, n> a;
            for (size_t i = 0; i < m*n; ++i) a.v[i] = -v[i];
            return a;
        }

        //Scalar multiplication.
        inline genmat<t, m, n> & operator *= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] *= a;
            return *this;
        }

        inline friend genmat<t, m, n> operator * (genmat<t, m, n> a, const t &b) noexcept {return a *= b;}
        inline friend genmat<t, m, n> operator * (const t &b, genmat<t, m, n> a) noexcept {return a *= b;}
        
        inline genmat<t, m, n> & operator /= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] /= a;
            return *this;
        }

        inline friend genmat<t, m, n> operator / (genmat<t, m, n> a, const t &b) noexcept {return a /= b;}
        inline friend genmat<t, m, n> operator / (const t &b, genmat<t, m, n> a) noexcept
        {
            genmat<t, m, n> c;
            for (size_t i = 0; i < m*n; ++i) c.v[i] = b/a.v[i];
            return c;
        }

        //Integer modulo.
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator %= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] %= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator %= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] %= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator % (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a %= b;}
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator %= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] %= a;
            return *this;
        }
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator % (genmat<t, m, n> a, const t &b) noexcept {return a %= b;}
        
        //Integer bit-wise and.
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator &= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] &= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator &= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] &= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator & (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a &= b;}
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator &= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] &= a;
            return *this;
        }
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator & (genmat<t, m, n> a, const t &b) noexcept {return a &= b;}
        
        //Integer bit-wise or.
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator |= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] |= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator |= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] |= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator | (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a |= b;}
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator |= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] |= a;
            return *this;
        }
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator | (genmat<t, m, n> a, const t &b) noexcept {return a |= b;}
        
        //Integer bit-wise shift right.
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator >>= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] >>= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator >>= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] >>= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator >> (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a >>= b;}
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator >>= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] >>= a;
            return *this;
        }
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator >> (genmat<t, m, n> a, const t &b) noexcept {return a >>= b;}
        
        //Integer bit-wise shift left.
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator <<= (const genmat<t, m, n> &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] <<= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator <<= (genmat<t, m, n> &&a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] <<= a.v[i];
            return *this;
        }

        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator << (genmat<t, m, n> a, const genmat<t, m, n> &b) noexcept {return a <<= b;}
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n> &>
        inline operator <<= (const t &a) noexcept
        {
            for (size_t i = 0; i < m*n; ++i) v[i] <<= a;
            return *this;
        }
        
        template<typename s = t>
        std::enable_if_t<std::is_integral<s>::value, genmat<t, m, n>>
        inline friend operator << (genmat<t, m, n> a, const t &b) noexcept {return a <<= b;}

        //Matrix multiplication.
        template<size_t k>
        inline genmat<t, m, k> operator * (const genmat<t, n, k> &a) const noexcept
        {
            genmat<t, m, k> b;
            
            for (size_t i = 0; i < m; ++i)
            {
                for (size_t j = 0; j < k; ++j)
                {
                    t s = t(0);

                    for (size_t l = 0; l < n; ++l)
                    {
                        s += (*this)(i, l)*a(l, j);
                    }

                    b(i, j) = s;
                }
            }
            
            return b;
        }
        
        //Transposition.
        inline genmat<t, n, m> transpose() const noexcept
        {
            genmat<t, n, m> a;

            for (size_t i = 0; i < m; ++i)
            {
                for (size_t j = 0; j < n; ++j)
                {
                    a(j, i) = (*this)(i, j);
                }
            }

            return a;
        }

        inline genmat<t, (m <= n ? m : n), 1> getDiagonal() const noexcept
        {
            genmat<t, (m <= n ? m : n), 1> a;

            for (size_t i = 0; i < std::min(m, n); ++i)
            {
                a(i, 0) = (*this)(i, i);
            }

            return a;
        }
        
        //Frobenius norm.
        friend t norm(const genmat<t, m, n> &a) noexcept
        {
            t f = t(0);
            for (size_t i = 0; i < m*n; ++i) f += a.v[i]*a.v[i];
            return std::sqrt(f);
        }
        
        //Create specific matrices.
        static genmat<t, m, n> Zero() noexcept
        {
            genmat<t, m, n> a;

            a.v.fill(t(0));

            return a;
        }

        static genmat<t, m, n> Identity() noexcept
        {
            genmat<t, m, n> a;

            a.v.fill(t(0));

            for (size_t i = 0; i < std::min(m, n); ++i)
            {
                a(i, i) = t(1);
            }

            return a;
        }

        friend std::ostream & operator << (std::ostream &Out, const genmat<t, m, n> &a)
        {
            Out << "(";
            
            for (size_t i = 0; i < m; ++i)
            {
                Out << a(i, 0);

                for (size_t j = 1; j < n; ++j)
                {
                    Out << ", " << a(i, j);
                }
                
                Out << std::endl;
            }

            Out << ")";

            return Out;
        }
};

//Generic column vector.
template<typename t, size_t m>
class genvec : public genmat<t, m, 1>
{
    public:
        genvec() = default;
        genvec(const genvec<t, m> &) = default;
        genvec(genvec<t, m> &&) = default;
        genvec<t, m> & operator = (const genvec<t, m> &) = default;
        genvec<t, m> & operator = (genvec<t, m> &&) = default;
        ~genvec() = default;
        
        //Specific constructors.
        template <typename ...Args>
        genvec(Args ...args) noexcept : genmat<t, m, 1>(args...) {}

        explicit genvec(const t &a) noexcept {this->v.fill(a);}

        //Coefficients.
        inline const t & operator () (const size_t i) const noexcept {return this->v[i];};
        inline t & operator () (const size_t i) noexcept {return this->v[i];};

        //Outer product.
        template <size_t n = m>
        typename std::enable_if_t<n == 3, genvec<t, m>>
        inline friend cross(const genvec<t, m> &a, const genvec<t, m> &b) noexcept
        {
            return genvec<t, m>(a.v[1]*b.v[2] - a.v[2]*b.v[1],
                                a.v[2]*b.v[0] - a.v[0]*b.v[2],
                                a.v[0]*b.v[1] - a.v[1]*b.v[0]);
        }

        //Inner products and lengths.
        inline friend t dot(const genvec<t, m> &a, const genvec<t, m> &b) noexcept
        {
            t d = t(0);
            for (size_t i = 0; i < m; ++i) d += a.v[i]*b.v[i];
            return d;
        }
        
        inline friend t length(const genvec<t, m> &a) noexcept {return std::sqrt(dot(a, a));}
        inline friend t length2(const genvec<t, m> &a) noexcept {return dot(a, a);}
        inline friend genvec<t, m> normalize(const genvec<t, m> &a) noexcept {return a/std::max<t>(length(a), EPS);}
        
        inline friend t minComponent(const genvec<t, m> &a) noexcept {return *std::min_element(a.v.begin(), a.v.end());}
        inline friend t maxComponent(const genvec<t, m> &a) noexcept {return *std::max_element(a.v.begin(), a.v.end());}
    
        //Hadamard element-wise product for vectors.
        //FIXME: This is dangerous since we use the same operator for matrix-multiplication!
        inline genvec<t, m> & operator *= (const genvec<t, m> &a) noexcept
        {
            for (size_t i = 0; i < m; ++i) this->v[i] *= a.v[i];
            return *this;
        }

        inline genvec<t, m> & operator *= (genvec<t, m> &&a) noexcept
        {
            for (size_t i = 0; i < m; ++i) this->v[i] *= a.v[i];
            return *this;
        }

        inline friend genvec<t, m> operator * (genvec<t, m> a, const genvec<t, m> &b) noexcept {return a *= b;}
        
        //Quaternion product.
        template <size_t n = m>
        typename std::enable_if_t<n == 4, genvec<t, m>>
        inline friend quatmul(const genvec<t, m> &a, const genvec<t, m> &b) noexcept
        {
            return genvec<t, m>(a.v[3]*b.v[0] + b.v[3]*a.v[0] + a.v[1]*b.v[2] - a.v[2]*b.v[1],
                                a.v[3]*b.v[1] + b.v[3]*a.v[1] + a.v[2]*b.v[0] - a.v[0]*b.v[2],
                                a.v[3]*b.v[2] + b.v[3]*a.v[2] + a.v[0]*b.v[1] - a.v[1]*b.v[0],
                                a.v[3]*b.v[3] - (a.v[0]*b.v[0] + a.v[1]*b.v[1] + a.v[2]*b.v[2]));
        }
        
        //Quaternion conjugation.
        template <size_t n = m>
        typename std::enable_if_t<n == 4, genvec<t, m>>
        inline friend quatconj(const genvec<t, m> &a) noexcept
        {
            return genvec<t, m>(-a.v[0], -a.v[1], -a.v[2], a.v[3]);
        }

        //Quaternion from axis-angle rotation.
        template <size_t n = m>
        typename std::enable_if_t<n == 4, genvec<t, m>>
        inline friend quatrot(const t &alpha, const genvec<t, 3> &a) noexcept
        {
            const t s = std::sin(t(0.5)*alpha);

            return genvec<t, m>(s*a.v[0], s*a.v[1], s*a.v[2], std::cos(t(0.5)*alpha));
        }


        //Access coefficients and swizzle vectors.
        inline t x() const noexcept {return this->v[0];}
        inline t y() const noexcept {if constexpr (m >= 2) {return this->v[1];} else {return t(0);}}
        inline t z() const noexcept {if constexpr (m >= 3) {return this->v[2];} else {return t(0);}}
        inline t w() const noexcept {if constexpr (m >= 4) {return this->v[3];} else {return t(0);}}

        inline genvec<t, 2> xy() const noexcept {if constexpr (m >= 2) {return genvec<t, 2>(this->v[0], this->v[1]);} else {return genvec<t, 2>(t(0));}}
        inline genvec<t, 2> yx() const noexcept {if constexpr (m >= 2) {return genvec<t, 2>(this->v[1], this->v[0]);} else {return genvec<t, 2>(t(0));}}
        
        inline genvec<t, 3> xyz() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[0], this->v[1], this->v[2]);} else {return genvec<t, 3>(t(0));}}
        inline genvec<t, 3> xzy() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[0], this->v[2], this->v[1]);} else {return genvec<t, 3>(t(0));}}
        inline genvec<t, 3> zxy() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[2], this->v[0], this->v[1]);} else {return genvec<t, 3>(t(0));}}
        inline genvec<t, 3> zyx() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[2], this->v[1], this->v[0]);} else {return genvec<t, 3>(t(0));}}
        inline genvec<t, 3> yzx() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[1], this->v[2], this->v[0]);} else {return genvec<t, 3>(t(0));}}
        inline genvec<t, 3> yxz() const noexcept {if constexpr (m >= 3) {return genvec<t, 3>(this->v[1], this->v[0], this->v[2]);} else {return genvec<t, 3>(t(0));}}
};

//Force instantiation.
template class genmat<float, 2, 2>;
template class genmat<float, 3, 3>;
template class genmat<float, 4, 4>;

template class genvec<float, 2>;
template class genvec<float, 3>;
template class genvec<float, 4>;

genmat<float, 3, 3> fromFixedSizeMatrix(const mat3 &a) noexcept
{
    return genmat<float, 3, 3>(a.v00, a.v10, a.v20,
                               a.v01, a.v11, a.v21,
                               a.v02, a.v12, a.v22);
}

mat3 toFixedSizeMatrix(const genmat<float, 3, 3> &a) noexcept
{
    return mat3(a(0, 0), a(1, 0), a(2, 0),
                a(0, 1), a(1, 1), a(2, 1),
                a(0, 2), a(1, 2), a(2, 2));
}

vec3 toFixedSizeVector(const genmat<float, 3, 1> &a) noexcept
{
    return vec3(a(0, 0), a(1, 0), a(2, 0));
}

//Perform eigendecomposition using Jacobi's algorithm for symmetric matrices.

//Algorithm 8.4.1 from Golub & van Loan's Matrix Computations.
//Computes givens rotation to zero a_{pq} and a_{qp} for symmetric A.
template<typename t, size_t n>
genmat<t, n, n> symSchur2(const genmat<t, n, n> &a, const size_t &p, const size_t &q)
{
    t c = t(1.0);
    t s = t(0.0);

    if (std::abs(a(p, q)) > EPS)
    {
        t r = (a(q, q) - a(p, p))/(t(2.0)*a(p, q));
        
        r = (r >= t(0.0) ?
                t(1.0)/(r + std::sqrt(t(1.0) + r*r)) :
                t(-1.0)/(-r + std::sqrt(t(1.0) + r*r)));

        c = t(1.0)/std::sqrt(t(1.0) + r*r);
        s = r*c;
    }
    
    //Create Givens rotation matrix. (FIXME: This can be done much more efficiently.)
    genmat<t, n, n> J = genmat<t, n, n>::Identity();

    J(p, p) = c;
    J(p, q) = s;
    J(q, p) = -s;
    J(q, q) = c;

    return J;
}

//Adaptation of algorithm 8.4.3 from Golub & van Loan's Matrix Computations.
//Computes eigenvalues and vectors of a symmetric matrix A using the cyclic Jacobi algorithm.
//Assumes matrix size # sweeps is sufficient.
template<typename t, size_t n>
std::tuple<genmat<t, n, 1>, genmat<t, n, n>> eigenDecompositionSym(genmat<t, n, n> a)
{
    //Start with identity matrix for eigenvectors.
    genmat<t, n, n> e = genmat<t, n, n>::Identity();
    
#ifndef NDEBUG
    const auto aCheck = a;
#endif

    for (size_t sweep = 0; sweep < n; ++sweep)
    {
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = i + 1; j < n; ++j)
            {
                const auto b = symSchur2(a, i, j);
                
                a = b.transpose()*a*b;
                e = e*b;
            }
        }
    }

#ifndef NDEBUG
    //Check that we have a diagonal matrix.
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            if (i != j)
            {
                assert(std::abs(a(i, j)) < EPS);
            }
        }
    }

    //Check that we found the eigenvectors.
    std::cout << std::scientific << norm(aCheck - e*a*e.transpose()) << " < " << std::scientific << EPS << "?" << std::endl;
    //FIXME: Can we achieve 1e-6 single precision for high-mass objects?
    assert(norm(aCheck - e*a*e.transpose()) < 1.0e-4);
#endif
    
    //Eigenvalues are on the diagonal.
    return {a.getDiagonal(), e};
}

}

