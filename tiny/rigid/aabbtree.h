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

#include <vector>
#include <list>
#include <map>
#include <set>

#include <tiny/math/vec.h>

namespace tiny
{

namespace aabb
{

struct aabb
{
    vec3 lb, ub;

    inline float getArea() const noexcept
    {
        vec3 d = max(ub - lb, vec3(0.0f));

        d = d*d.yzx();

        return 2.0f*(d.x + d.y + d.z);
    }

    inline bool isSubsetOf(const aabb &b) const noexcept
    {
        return (lb.x >= b.lb.x) && (ub.x <= b.ub.x) &&
               (lb.y >= b.lb.y) && (ub.y <= b.ub.y) &&
               (lb.z >= b.lb.z) && (ub.z <= b.ub.z);
    }

    inline aabb scale(const float &s) const noexcept
    {
        const vec3 d = 0.5f*(ub - lb);
        const vec3 m = 0.5f*(ub + lb);

        return aabb{m - s*d, m + s*d};
    }

    inline friend aabb cup(const aabb &a, const aabb &b) noexcept
    {
        return aabb{min(a.lb, b.lb), max(a.ub, b.ub)};
    }

    inline friend bool overlapping(const aabb &a, const aabb &b) noexcept
    {
        return (a.ub.x >= b.lb.x) && (b.ub.x >= a.lb.x) &&
               (a.ub.y >= b.lb.y) && (b.ub.y >= a.lb.y) &&
               (a.ub.z >= b.lb.z) && (b.ub.z >= a.lb.z);
    }
};

struct Node
{
    aabb box;
    int parent, child1, child2;
    int contents;

    inline bool isLeaf() const noexcept
    {
        return child1 < 0;
    }
};

class Tree
{
    public:
        Tree();
        ~Tree();
        
        void clear();
        bool insert(const aabb &, const int &);
        bool erase(const int &);
        aabb getNodeBox(const int &) const noexcept;
        float getCost() const;
        void check() const;
        std::set<std::pair<int, int>> getOverlappingContents() const noexcept;

        inline size_t size() const noexcept
        {
            return contentsToLeaf.size();
        }
        
    private:
        int insertNode(const Node &);
        void eraseNode(const int &);
        void rebalance(const int &);

        std::vector<Node> nodes;
        //TODO: Is an std::queue faster?
        std::vector<int> freeNodes;
        std::map<int, int> contentsToLeaf;
        int root;
};

} //aabb

} //tiny

