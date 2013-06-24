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
#include <queue>

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
            
            std::vector<vec3> temporaryPositions(first, last);
            std::vector<int> temporaryInstances(temporaryPositions.size());
            
            std::cerr << "Constructing quadtree of " << temporaryInstances.size() << " objects..." << std::endl;
            
            for (unsigned int i = 0; i < temporaryInstances.size(); ++i)
            {
                temporaryInstances[i] = i;
            }
            
            instances = temporaryInstances;
            nodes.push_back(QuadtreeNode(0, temporaryInstances.size()));
            splitNode(nodes[0], temporaryInstances, temporaryPositions);

#ifndef NDEBUG
            //Check that we have really created a proper permutation.
            if (true)
            {
                std::vector<bool> check(instances.size(), false);
                
                for (unsigned int i = 0; i < instances.size(); ++i)
                {
                    assert(instances[i] < 0 || instances[i] >= instances.size());
                    check[instances[i]] = true;
                }
                
                for (std::vector<bool>::const_iterator i = check.begin(); i != check.end(); ++i)
                {
                    assert(*i);
                }
            }
#endif
            
            std::cerr << "Built quadtree with " << nodes.size() << " nodes." << std::endl;
        }
        
        template <typename Iterator>
        Iterator retrieveIndicesBetweenRadii(const vec3 &position, const float &minRadius, const float &maxRadius, Iterator indices, int maxNrIndices) const
        {
            if (instances.empty())
            {
                std::cerr << "Warning: Unable to determine indices within an empty quadtree!" << std::endl;
                return indices;
            }
            
            if (maxNrIndices <= 0)
            {
                std::cerr << "Warning: Filling empty instance list!" << std::endl;
                return indices;
            }
            
            //Recursively traverse the quadtree to find the indices of all objects such that their distance lies between the radii.
            std::queue<int> queue;
            
            queue.push(0);
            
            while (!queue.empty() && maxNrIndices > 0)
            {
                const QuadtreeNode n = nodes[queue.front()];
                
                queue.pop();
                
                const float distance = length(n.centre - position);
                const bool hasChildren = (n.children[0] != 0 || n.children[1] != 0 || n.children[2] != 0 || n.children[3] != 0);
                
                if (distance + n.radius < minRadius || distance - n.radius >= maxRadius)
                {
                    //The node is completely outside the annulus.
                    continue;
                }
                else if (distance - n.radius >= minRadius && distance + n.radius <= maxRadius)
                {
                    //The node is contained entirely within the annulus.
                    for (int i = n.startIndex; i < n.endIndex && maxNrIndices > 0; ++i)
                    {
                        *indices++ = instances[i];
                        --maxNrIndices;
                    }
                }
                else if (hasChildren)
                {
                    //Partially contained node with children, so we recurse.
                    for (int i = 0; i < 4; ++i)
                    {
                        if (n.children[i] != 0)
                        {
                            queue.push(n.children[i]);
                        }
                    }
                }
                else if (distance >= minRadius && distance < maxRadius)
                {
                    //Indivisible partially contained node.
                    for (int i = n.startIndex; i < n.endIndex && maxNrIndices > 0; ++i)
                    {
                        *indices++ = instances[i];
                        --maxNrIndices;
                    }
                }
            }
            
            return indices;
        }
        
    private:
        void splitNode(QuadtreeNode &, std::vector<int> &, const std::vector<vec3> &);
        
        std::vector<int> instances;
        float instanceRadius;
        std::vector<QuadtreeNode> nodes;
};

}

}

