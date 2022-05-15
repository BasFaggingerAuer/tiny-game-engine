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

#include <cassert>

#include <tiny/math/vec.h>

//(Slow, should use LAPACK/Eigen/..., but this code will only be used incidentally and its fun to figure this out.)
namespace tiny
{

//Generic matrix class.
template<typename t, size_t m, size_t n>
class genmat
{
    public:
        genmat() = default;
        genmat(const genmat &) = default;
        ~genmat() = default;
        
        inline const t & operator () (const size_t i, const size_t j) const noexcept {assert(i < m && j < n); return v[m*j + i];};
        inline t & operator () (const size_t i, const size_t j) noexcept {assert(i < m && j < n); return v[m*j + i];};

        template<size_t k>
        genmat<t, m, k> operator * (const genmat<t, n, k> &a) const noexcept
        {
            genmat<t, m, k> b;
            
            for (size_t i = 0; i < m; ++i)
            {
                for (size_t j = 0; j < k; ++j)
                {
                    t s = t(0.0);

                    for (size_t l = 0; l < n; ++l)
                    {
                        s += (*this)(i, l)*a(l, j);
                    }

                    b(i, j) = s;
                }
            }
            
            return b;
        }

        genmat<t, n, m> transpose() const noexcept
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

        genmat<t, m, n> operator + (const genmat<t, m, n> &a) const noexcept
        {
            genmat<t, m, n> b;

            for (size_t i = 0; i < m*n; ++i)
            {
                b.v[i] = v[i] + a.v[i];
            }

            return b;
        }

        genmat<t, m, n> operator - (const genmat<t, m, n> &a) const noexcept
        {
            genmat<t, m, n> b;

            for (size_t i = 0; i < m*n; ++i)
            {
                b.v[i] = v[i] - a.v[i];
            }

            return b;
        }

        genmat<t, m, n> operator * (const t &a) const noexcept
        {
            genmat<t, m, n> b;

            for (size_t i = 0; i < m*n; ++i)
            {
                b.v[i] = v[i]*a;
            }

            return b;
        }

        genmat<t, m, n> operator / (const t &a) const noexcept
        {
            genmat<t, m, n> b;

            for (size_t i = 0; i < m*n; ++i)
            {
                b.v[i] = v[i]/a;
            }

            return b;
        }

        genmat<t, (m <= n ? m : n), 1> getDiagonal() const noexcept
        {
            genmat<t, (m <= n ? m : n), 1> a;

            for (size_t i = 0; i < std::min(m, n); ++i)
            {
                a(i, 0) = (*this)(i, i);
            }

            return a;
        }

        t getFrobeniusNorm() const noexcept
        {
            t f = 0.0;

            for (size_t i = 0; i < m*n; ++i)
            {
                f += v[i]*v[i];
            }

            return std::sqrt(f);
        }

        static genmat<t, m, n> Zero() noexcept
        {
            genmat<t, m, n> a;

            a.v.fill(t(0.0));

            return a;
        }

        static genmat<t, m, n> Identity() noexcept
        {
            genmat<t, m, n> a;

            a.v.fill(t(0.0));

            for (size_t i = 0; i < std::min(m, n); ++i)
            {
                a(i, i) = t(1.0);
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

        std::array<t, m*n> v;
};

template<typename t, size_t m, size_t n>
genmat<t, m, n> operator * (const t &a, const genmat<t, m, n> &b) noexcept
{
    genmat<t, m, n> c;

    for (size_t i = 0; i < m*n; ++i)
    {
        c.v[i] = a*b.v[i];
    }

    return b;
}

template<typename t, size_t m, size_t n>
genmat<t, m, n> operator / (const t &a, const genmat<t, m, n> &b) noexcept
{
    genmat<t, m, n> c;

    for (size_t i = 0; i < m*n; ++i)
    {
        c.v[i] = a/b.v[i];
    }

    return b;
}

genmat<float, 3, 3> fromFixedSizeMatrix(const mat3 &a) noexcept
{
    return genmat<float, 3, 3>({{a.v00, a.v10, a.v20,
                                 a.v01, a.v11, a.v21,
                                 a.v02, a.v12, a.v22}});
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
    assert((aCheck - e*a*e.transpose()).getFrobeniusNorm() < EPS);
#endif
    
    //Eigenvalues are on the diagonal.
    return {a.getDiagonal(), e};
}

}

