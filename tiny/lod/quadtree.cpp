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
#include <tiny/lod/quadtree.h>

using namespace tiny::lod;

Quadtree::Quadtree() :
    instances(),
    instancePositions(),
    nodes()
{

}

Quadtree::~Quadtree()
{

}

void Quadtree::splitNode(QuadtreeNode &node, std::vector<int> &idx, const std::vector<vec3> &pos)
{
    //Discard empty nodes.
    if (node.startIndex >= node.endIndex)
    {
        return;
    }
    
    //Determine the bounding box of this node.
    vec3 minBound = pos[idx[node.startIndex]];
    vec3 maxBound = pos[idx[node.startIndex]];
    
    for (int i = node.startIndex + 1; i < node.endIndex; ++i)
    {
        minBound = min(minBound, pos[idx[i]]);
        maxBound = max(maxBound, pos[idx[i]]);
    }
    
    //Determine the node's centre and radius.
    node.centre = 0.5f*(minBound + maxBound);
    node.radius = 0.0f;
    
    for (int i = node.startIndex; i < node.endIndex; ++i)
    {
        const float r = length(pos[idx[i]] - node.centre);
        
        node.radius = std::max(node.radius, r);
    }
    
    //Clear children for this node.
    for (int i = 0; i < 4; ++i)
    {
        node.children[i] = 0;
    }
    
    //Do we want to subdivide this node further?
    if (node.endIndex - node.startIndex <= 4)
    {
        return;
    }
    
    //Yes, count the number of objects in each leaf.
    int counts[4] = {0, 0, 0, 0};
    const vec3 invScale = vec3(1.0f/(maxBound.x - minBound.x), 1.0f, 1.0f/(maxBound.z - minBound.z));
    
    for (int i = node.startIndex; i < node.endIndex; ++i)
    {
        const vec3 v = (pos[idx[i]] - minBound)*invScale;
        const int leaf = (v.x < 0.5f ? 0 : 1) | (v.z < 0.5f ? 0 : 2);
        
        counts[leaf]++;
    }
    
    assert(node.startIndex + counts[0] + counts[1] + counts[2] + counts[3] == node.endIndex);
    
    //Create new ranges for the leaves.
    int offsets[4] = {
        node.startIndex,
        node.startIndex + counts[0],
        node.startIndex + counts[0] + counts[1],
        node.startIndex + counts[0] + counts[1] + counts[2]};
    
    //Perform a counting sort.
    for (int i = node.startIndex; i < node.endIndex; ++i)
    {
        const vec3 v = (pos[idx[i]] - minBound)*invScale;
        const int leaf = (v.x < 0.5f ? 0 : 1) | (v.z < 0.5f ? 0 : 2);
        
        instances[offsets[leaf]++] = idx[i];
    }
    
    assert(offsets[0] == node.startIndex + counts[0]);
    assert(offsets[1] == node.startIndex + counts[0] + counts[1]);
    assert(offsets[2] == node.startIndex + counts[0] + counts[1] + counts[2]);
    assert(offsets[3] == node.endIndex);
    
    //Copy the sorted indices.
    for (int i = node.startIndex; i < node.endIndex; ++i)
    {
        idx[i] = instances[i];
    }
    
    //Create leaves for all non-empty nodes.
    int offset = node.startIndex;
    
    for (int i = 0; i < 4; ++i)
    {
        if (counts[i] > 0)
        {
            node.children[i] = nodes.size();
            nodes.push_back(QuadtreeNode(offset, offset + counts[i]));
            offset += counts[i];
        }
        else
        {
            node.children[i] = 0;
        }
    }
    
    //Recurse on all child nodes.
    for (int i = 0; i < 4; ++i)
    {
        if (node.children[i] > 0)
        {
            splitNode(nodes[node.children[i]], idx, pos);
        }
    }
}

