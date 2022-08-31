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
#include <cassert>
#include <queue>
#include <stack>
#include <algorithm>

#include <tiny/rigid/aabbtree.h>

using namespace tiny::aabb;

//Based on Dynamic Bounding Volume Hierarchies by Erin Catto.

Tree::Tree()
{
    clear();
}

Tree::~Tree()
{

}

void Tree::clear()
{
    nodes.clear();
    freeNodes.clear();
    contentsToLeaf.clear();
    root = -1;
}

int Tree::insertNode(const Node &n)
{
    int i;

    if (freeNodes.empty())
    {
        i = nodes.size();
        nodes.push_back(n);
    }
    else
    {
        i = freeNodes.back();
        freeNodes.pop_back();
        nodes[i] = n;
    }

    return i;
}

void Tree::eraseNode(const int &i)
{
    //The user is responsible for disconnecting this node from the tree.
    freeNodes.push_back(i);
}

float Tree::getCost() const
{
    float area = 0.0f;

    for (const auto &n : nodes)
    {
        area += n.box.getArea();
    }

    return area;
}

aabb Tree::getNodeBox(const int &contents) const noexcept
{
    //Get bounding box of an existing node.
    auto ptr = contentsToLeaf.find(contents);

    assert(ptr != contentsToLeaf.end());

    return nodes[ptr->second].box;
}

std::set<std::pair<int, int>> Tree::getOverlappingContents() const noexcept
{
    //Get all pairs of overlapping leaf AABBs, indexed by their contents.
    std::set<std::pair<int, int>> pairs;

    for (const auto & [c, i] : contentsToLeaf)
    {
        const aabb b = nodes[i].box;

        std::stack<int> s;

        s.push(root);

        while (!s.empty())
        {
            const auto n = nodes[s.top()];
            s.pop();

            if (n.isLeaf())
            {
                //By earlier checks we should overlap with this node.
                assert(overlapping(b, n.box));
                pairs.insert(std::minmax(c, n.contents));
            }
            else
            {
                //Does it make sense to recurse further?
                if (overlapping(b, nodes[n.child1].box)) s.push(n.child1);
                if (overlapping(b, nodes[n.child2].box)) s.push(n.child2);
            }
        }
    }

    return pairs;
}

void Tree::check() const
{
    //Check whether the tree is OK.
    //std::cout << "CHECKING TREE WITH " << size() << " NODES AND ROOT (" << root << "):" << std::endl;
    
    if (root < 0)
    {
        assert(size() == 0);
        assert(contentsToLeaf.empty());
        assert(nodes.size() == freeNodes.size());
        return;
    }

    std::queue<int> q;
    
    /*
    q.push(root);

    while (!q.empty())
    {
        //Get tree node.
        const auto i = q.front();
        const auto n = nodes[i];
        q.pop();

        std::cout << i << " (cont. " << n.contents << ", par. " << n.parent << ")";

        if (!n.isLeaf())
        {
            std::cout << " --> " << n.child1 << ", " << n.child2;
            q.push(n.child1);
            q.push(n.child2);
        }

        std::cout << std::endl;
    }
    */

    assert(static_cast<size_t>(root) < nodes.size());
    assert(nodes[root].parent == -1);
    
    //Leaves should exactly be the content nodes.
    for (const auto & [c, i] : contentsToLeaf)
    {
        assert(i >= 0 && static_cast<size_t>(i) < nodes.size());
        assert(nodes[i].isLeaf());
    }

    //Check tree structure.
    std::vector<bool> used(nodes.size(), false);
    
    q = std::queue<int>();
    q.push(root);

    while (!q.empty())
    {
        //Get tree node.
        const auto i = q.front();
        assert(i >= 0);
        const auto n = nodes[i];

        q.pop();

        //Should not be a free node.
        used[i] = true;
        
        //Either be the root node or have a parent.
        assert((i == root) != (n.parent >= 0));
        
        //Check leaves.
        if (n.isLeaf())
        {
            assert(contentsToLeaf.at(n.contents) == i);
            assert(n.child1 < 0 && n.child2 < 0);
        }

        //Check non-leaves.
        if (n.child1 >= 0)
        {
            assert(n.child2 >= 0);
            assert(nodes[n.child1].parent == i);
            assert(!n.isLeaf());
            q.push(n.child1);
        }
        if (n.child2 >= 0)
        {
            assert(n.child1 >= 0);
            assert(nodes[n.child2].parent == i);
            assert(!n.isLeaf());
            q.push(n.child2);
        }
    }
    
    //Check that we properly keep track of the unused nodes.
    std::vector<bool> unused(nodes.size(), false);
    
    for (const auto &i : freeNodes)
    {
        assert(i >= 0 && static_cast<size_t>(i) < nodes.size());
        assert(!used[i]);
        unused[i] = true;
    }

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        assert(used[i] == !unused[i]);
    }
}

bool Tree::insert(const aabb &box, const int &contents)
{
    //Insert a node into the AABB tree.
    //std::cout << "INSERT " << contents << std::endl;

    check();
    
    //Do we already have this node?
    if (contentsToLeaf.find(contents) != contentsToLeaf.end())
    {
        return false;
    }
    
    const int newNode = insertNode({box, -1, -1, -1, contents});
    contentsToLeaf.insert({contents, newNode});

    //Was the tree empty?
    if (root == -1)
    {
        root = newNode;
        assert(contentsToLeaf.size() == 1);
        return true;
    }

    //Find optimal position in tree for the new node.
    int bestSibling = root;
    float bestCost = cup(nodes[bestSibling].box, nodes[newNode].box).getArea();
    const float minCost = nodes[newNode].box.getArea();
    //Create a min-heap.
    std::priority_queue<std::pair<float, int>,
                        std::vector<std::pair<float, int>>,
                        std::greater<std::pair<float, int>>> queue;
    
    queue.push({0.0f, root});

    while (!queue.empty())
    {
        auto [c, i] = queue.top();
        
        queue.pop();

        //Exact cost of adding newNode at node i.
        c += cup(nodes[i].box, nodes[newNode].box).getArea();

        if (c < bestCost)
        {
            bestCost = c;
            bestSibling = i;
        }

        //See whether we will recurse.
        if (!nodes[i].isLeaf())
        {
            //Look at increase of area at node i if we add newNode to one of i's children.
            c -= nodes[i].box.getArea();
            
            //Do we have a chance to improve?
            if (c + minCost < bestCost)
            {
                queue.push({c, nodes[i].child1});
                queue.push({c, nodes[i].child2});
            }
        }
    }

    //Create a new parent.
    const int oldParent = nodes[bestSibling].parent;
    const int newParent = insertNode({box, oldParent, newNode, bestSibling, -1});

    nodes[newNode].parent = newParent;
    nodes[bestSibling].parent = newParent;

    if (oldParent < 0)
    {
        //We are a sibling of the previous root node.
        assert(bestSibling == root);
        root = newParent;
    }
    else
    {
        //We are the sibling of a node inside the tree.
        if (nodes[oldParent].child1 == bestSibling)
        {
            nodes[oldParent].child1 = newParent;
        }
        else
        {
            assert(nodes[oldParent].child2 == bestSibling);
            nodes[oldParent].child2 = newParent;
        }
    }

    //Refit and rebalance tree.
    int index = newParent;

    while (index >= 0)
    {
        nodes[index].box = cup(nodes[nodes[index].child1].box,
                               nodes[nodes[index].child2].box);
        rebalance(index);
        index = nodes[index].parent;
    }

    return true;
}

void Tree::rebalance(const int &a)
{
    //Minimize tree cost by rotating children.
    //
    //       A
    //     /   \    |
    //    B     C
    //   / \   / \  |
    //  D   E F   G
    //
    
    if (nodes[a].isLeaf()) return;

    const int b = nodes[a].child1;
    const int c = nodes[a].child2;

    //std::cout << "REBALANCE " << a << " (" << nodes[a].parent << ") --> "
    //                          << b << " (" << nodes[b].parent << "), "
    //                          << c << " (" << nodes[c].parent << ")" << std::endl;

    if (!nodes[b].isLeaf())
    {
        const int d = nodes[b].child1;
        const int e = nodes[b].child2;
        // (I)
        //       A
        //     /   \  |
        //    B     C
        //   / \      |
        //  D   E
        float area1 = nodes[b].box.getArea();
        // (II)
        //       A
        //     /   \  |
        //    B     D
        //   / \      |
        //  C   E
        aabb b2 = cup(nodes[c].box, nodes[e].box);
        float area2 = b2.getArea();
        // (III)
        //       A
        //     /   \  |
        //    B     E
        //   / \      |
        //  D   C
        aabb b3 = cup(nodes[c].box, nodes[d].box);
        float area3 = b3.getArea();
        
        if (area2 <= area3 && area2 < area1)
        {
            // (II) is best.
            nodes[b].box = b2;
            nodes[b].child1 = c;
            nodes[c].parent = b;
            nodes[a].child2 = d;
            nodes[d].parent = a;
        }
        else if (area3 <= area2 && area3 < area1)
        {
            // (III) is best.
            nodes[b].box = b3;
            nodes[b].child2 = c;
            nodes[c].parent = b;
            nodes[a].child2 = e;
            nodes[e].parent = a;
        }
    }
    
    if (!nodes[c].isLeaf())
    {
        const int f = nodes[c].child1;
        const int g = nodes[c].child2;
        // (I)
        //       A
        //     /   \   |
        //    B     C
        //         / \ |
        //        F   G
        float area1 = nodes[c].box.getArea();
        // (II)
        //       A
        //     /   \   |
        //    F     C
        //         / \ |
        //        B   G
        aabb c2 = cup(nodes[b].box, nodes[g].box);
        float area2 = c2.getArea();
        // (III)
        //       A
        //     /   \   |
        //    G     C
        //         / \ |
        //        F   B
        aabb c3 = cup(nodes[b].box, nodes[f].box);
        float area3 = c3.getArea();
        
        if (area2 <= area3 && area2 < area1)
        {
            // (II) is best.
            nodes[c].box = c2;
            nodes[c].child1 = b;
            nodes[b].parent = c;
            nodes[a].child1 = f;
            nodes[f].parent = a;
        }
        else if (area3 <= area2 && area3 < area1)
        {
            // (III) is best.
            nodes[c].box = c3;
            nodes[c].child2 = b;
            nodes[b].parent = c;
            nodes[a].child1 = g;
            nodes[g].parent = a;
        }
    }
}

bool Tree::erase(const int &contents)
{
    //std::cout << "ERASE " << contents << std::endl;
    //Remove a leaf node from the AABB tree.
    
    //We should already have this node.
    auto iter = contentsToLeaf.find(contents);

    if (iter == contentsToLeaf.end())
    {
        return false;
    }
    
    //All content nodes should be leaves.
    //
    //  D              D
    //   \              \  |
    //    C     -->      B
    //   / \               |
    //  B   A
    //
    const int a = iter->second;

    assert(nodes[a].isLeaf());
    
    if (a == root)
    {
        //We are removing the root node.
        assert(size() == 1);
        clear();
        return true;
    }

    //Remove parent node.
    const int c = nodes[a].parent;
    const int b = (nodes[c].child1 == a ? nodes[c].child2 : nodes[c].child1);
    
    assert(c >= 0);
    assert(nodes[b].parent == c);
    assert(b != a);
    
    const int d = nodes[c].parent;

    nodes[b].parent = d;

    if (d >= 0)
    {
        if (nodes[d].child1 == c)
        {
            nodes[d].child1 = b;
        }
        else
        {
            assert(nodes[d].child2 == c);
            nodes[d].child2 = b;
        }
    }
    else
    {
        //C is the root node, so replace by B.
        root = b;
    }
    
    eraseNode(c);
    eraseNode(a);
    contentsToLeaf.erase(iter);

    return true;
}

