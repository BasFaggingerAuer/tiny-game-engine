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
#include <exception>
#include <string>
#include <vector>

#include <cassert>

#include <tiny/math/vec.h>

namespace tiny
{

namespace lod
{

struct QuadtreeNode
{
    QuadtreeNode() :
        startIndex(0),
        endIndex(0),
        radius(0.0f),
        centre(0.0f, 0.0f, 0.0f)
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i] = 0;
        }
    }
    
    QuadtreeNode(const int &a_startIndex, const int &a_endIndex) :
        startIndex(a_startIndex),
        endIndex(a_endIndex),
        radius(0.0f),
        centre(0.0f, 0.0f, 0.0f)
    {
        for (int i = 0; i < 4; ++i)
        {
            children[i] = 0;
        }
    }
    
    int children[4];
    int startIndex, endIndex;
    float radius;
    vec3 centre;
};

class Quadtree
{
    public:
        Quadtree();
        ~Quadtree();
        
        template <typename Iterator>
        void buildQuadtree(Iterator first, Iterator last, const float &radius)
        {
            instances.clear();
            nodes.clear();
            
            if (last - first <= 0)
            {
                std::cerr << "Warning: Empty quadtree!" << std::endl;
                return;
            }
            
            std::vector<vec3> unorderedInstances(first, last);
            
            std::cerr << "Constructing quadtree of " << unorderedInstances.size() << " objects..." << std::endl;
            
            instances = unorderedInstances;
            nodes.push_back(QuadtreeNode(0, unorderedInstances.size()));
            splitNode(nodes[0], unorderedInstances);
            
            std::cerr << "Built quadtree with " << nodes.size() << " nodes." << std::endl;
        }
        
    private:
        void splitNode(QuadtreeNode &, const std::vector<vec3> &);
        
        std::vector<vec3> instances;
        float instanceRadius;
        std::vector<QuadtreeNode> nodes;
};

}

}

