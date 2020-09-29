/*
Copyright 2020, Bas Fagginger Auer.

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
#include <iostream>
#include <algorithm>
#include <set>
#include <cassert>

#include <tiny/img/io/image.h>

#include "voxel.h"

using namespace rpg;
using namespace tiny;

//For voxel map compression/decompression.
struct HuffmanNode
{
    HuffmanNode() :
        symbols(),
        bits(),
        count(0),
        left(-1),
        right(-1)
    {
        
    }
    
    HuffmanNode(const uint8_t &a_symbol, const uint32_t &a_count) :
        symbols(),
        bits(),
        count(a_count),
        left(-1),
        right(-1)
    {
        symbols.insert(a_symbol);
    }
    
    HuffmanNode(const HuffmanNode &a, const HuffmanNode &b) :
        symbols(a.symbols),
        bits(),
        count(a.count + b.count),
        left(-1),
        right(-1)
    {
        symbols.insert(b.symbols.cbegin(), b.symbols.cend());
    }
    
    bool leaf() const
    {
        return (symbols.size() == 1);
    }
    
    bool operator < (const HuffmanNode &a) const
    {
        return count < a.count;
    }
    
    //Following https://stackoverflow.com/questions/759707/efficient-way-of-storing-huffman-tree.
    void treeToBits(std::vector<HuffmanNode> &tree, std::vector<bool> &treeBits, std::vector<std::vector<bool> > &symbolBits)
    {
        std::cout << "TTB:";
        
        for (auto i = symbols.cbegin(); i != symbols.cend(); ++i)
        {
            std::cout << " " << static_cast<int>(*i);
        }
        
        std::cout << " (bits ";
        
        for (auto i = bits.cbegin(); i != bits.cend(); ++i)
        {
            std::cout << (*i ? "1" : "0");
        }
        
        std::cout << ") --> " << left << ", " << right << std::endl;
        
        if (symbols.size() == 1)
        {
            //Leaf node.
            treeBits.push_back(true);
            
            const uint8_t v = *symbols.begin();
            
            for (uint32_t i = 0u; i < 8u; ++i)
            {
                treeBits.push_back((v & (1u << i)) != 0u);
            }
            
            symbolBits[v] = bits;
        }
        else
        {
            //Non-leaf node.
            treeBits.push_back(false);
            
            tree[left].bits = bits;
            tree[left].bits.push_back(false);
            tree[left].treeToBits(tree, treeBits, symbolBits);
            tree[right].bits = bits;
            tree[right].bits.push_back(true);
            tree[right].treeToBits(tree, treeBits, symbolBits);
        }
    }
    
    size_t bitsToTree(const std::vector<bool> &treeBits, size_t bp, std::vector<HuffmanNode> &tree, std::vector<std::vector<bool> > &symbolBits)
    {
        std::cout << "BTT (" << bp << "/" << treeBits.size() << " bits, " << tree.size() << " nodes):";
        
        for (auto i = symbols.cbegin(); i != symbols.cend(); ++i)
        {
            std::cout << " " << static_cast<int>(*i);
        }
        
        std::cout << " (bits ";
        
        for (auto i = bits.cbegin(); i != bits.cend(); ++i)
        {
            std::cout << (*i ? "1" : "0");
        }
        
        std::cout << ") ";
        
        if (bp >= treeBits.size())
        {
            std::cerr << "Unable to decode Huffman tree bits!" << std::endl;
            return treeBits.size();
        }
        
        if (treeBits[bp++])
        {
            //Leaf node.
            std::cout << "LEAF" << std::endl;
            
            uint8_t v = 0;
            
            for (uint32_t i = 0u; i < 8u; ++i)
            {
                v |= (treeBits[bp++] ? (1u << i) : 0u);
            }
            
            if (v >= symbolBits.size())
            {
                std::cerr << "Unexpected Huffman tree symbol!" << std::endl;
                return treeBits.size();
            }
            
            symbols.insert(v);
            symbolBits[v] = bits;
        }
        else
        {
            //Non-leaf node.
            std::cout << "NODE (OWN BITS ";
        for (auto i = bits.cbegin(); i != bits.cend(); ++i)
        {
            std::cout << (*i ? "1" : "0");
        }
            std::cout << ")" << std::endl;
            
            left = tree.size();
            tree.push_back(HuffmanNode());
            for (auto i = bits.cbegin(); i != bits.cend(); ++i) tree[left].bits.push_back(*i);
            tree[left].bits.push_back(false);
            std::cout << "  --> DIVE TO LEFT " << left << " WITH BITS ";
        for (auto i = tree[left].bits.cbegin(); i != tree[left].bits.cend(); ++i)
        {
            std::cout << (*i ? "1" : "0");
        }
            std::cout << std::endl;
            bp = tree[left].bitsToTree(treeBits, bp, tree, symbolBits);
            
            right = tree.size();
            tree.push_back(HuffmanNode());
            tree[right].bits = bits;
            tree[right].bits.push_back(true);
            bp = tree[right].bitsToTree(treeBits, bp, tree, symbolBits);
        }
        
        return bp;
    }
    
    std::set<uint8_t> symbols;
    std::vector<bool> bits;
    uint32_t count;
    int left, right;
};

GameVoxelMap::GameVoxelMap(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading voxel map resources..." << std::endl;
    
    assert(std::string(el->Value()) == "voxelmap");
    
    int width = 64;
    int height = 64;
    int depth = 64;
    float voxelSize = 1.0f;
    
    el->QueryIntAttribute("width", &width);
    el->QueryIntAttribute("height", &height);
    el->QueryIntAttribute("depth", &depth);
    el->QueryFloatAttribute("scale", &voxelSize);
    
    //Read cubemaps.
    std::vector<img::Image> cubeMaps;
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (std::string(sl->Value()) == "cubemap")
        {
            std::string textureFileName = "";
            
            sl->QueryStringAttribute("all", &textureFileName);
            
            if (!textureFileName.empty())
            {
                auto texture = img::io::readImage(path + textureFileName);
                
                for (int im = 0; im < 6; ++im)
                {
                    cubeMaps.push_back(texture);
                }
            }
            else
            {
                sl->QueryStringAttribute("px", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mx", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("py", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("my", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("pz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
                sl->QueryStringAttribute("mz", &textureFileName);
                cubeMaps.push_back(img::io::readImage(path + textureFileName));
            }
        }
        else
        {
            std::cerr << "Warning: unknown data " << sl->Value() << " encountered in XML!" << std::endl;
        }
    }
    
    voxelCubeArrayTexture = new draw::RGBTexture2DCubeArray(cubeMaps.begin(), cubeMaps.end(), draw::tf::repeat | draw::tf::mipmap);
    voxelMap = new draw::VoxelMap(std::max(width, std::max(height, depth))*4);
    voxelTexture = new draw::RGTexture3D(width, height, depth, draw::tf::none);
    
    //Create checkerboard pattern.
    for (size_t z = 0; z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 0; y < voxelTexture->getHeight(); ++y)
        {
            for (size_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 1] = (((x ^ y ^ z) & 1) == 0 ? 116 : 140);
            }
        }
    }
    
    voxelTexture->sendToDevice();
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    voxelMap->setCubeMaps(*voxelCubeArrayTexture);
    setVoxelBasePlane(1);
    createVoxelPalette();
}

GameVoxelMap::~GameVoxelMap()
{
    delete voxelMap;
    delete voxelTexture;
    delete voxelCubeArrayTexture;
}

void GameVoxelMap::createFromCompressedVoxels(const std::string &text)
{
    //Create voxels from output of getCompressedVoxels().
    
    std::cout << "MAPPING TO BITS" << std::endl;
    
    //Map ASCII to 6-bit blocks.
    std::vector<bool> bits;
    
    for (auto i = text.cbegin(); i != text.cend(); ++i)
    {
        uint32_t v = static_cast<uint32_t>(*i);
        
        if (v >= 42u && v < 44u)
        {
            v = v + 62u - 42u;
        }
        else if (v >= 48u && v < 58u)
        {
            v = v + 52u - 48u;
        }
        else if (v >= 65u && v < 91u)
        {
            v = v + 26u - 65u;
        }
        else if (v >= 97u && v < 123u)
        {
            v = v - 97u;
        }
        else
        {
            std::cerr << "Unable to decode ASCII symbols to bits!" << std::endl;
            return;
        }
        
        assert(v < 64u);
        
        for (uint32_t j = 0; j < 6u; ++j)
        {
            bits.push_back((v & (1u << j)) != 0u);
        }
    }
    
    uint32_t width = 0;
    size_t bp = 0;
    
    for (uint32_t j = 0; j < 32u; ++j)
    {
        width |= (bits[bp++] ? (1u << j) : 0u);
    }
    
    std::cout << "VOXEL DATA HAS SIZE " << width << std::endl;
    
    uint32_t nrVoxelTypes = 0;

    for (uint32_t j = 0; j < 8u; ++j)
    {
        nrVoxelTypes |= (bits[bp++] ? (1u << j) : 0u);
    }
    
    std::cout << nrVoxelTypes << " VOXEL TYPES" << std::endl;

    std::vector<HuffmanNode> tree;
    std::vector<std::vector<bool> > voxelTypeBits(nrVoxelTypes, std::vector<bool>());
    
    std::cout << "READING TREE..." << std::endl;
    
    tree.push_back(HuffmanNode());
    bp = tree.front().bitsToTree(bits, bp, tree, voxelTypeBits);
    
    if (bp >= bits.size() || voxelTypeBits.size() != nrVoxelTypes)
    {
        std::cerr << "Unable to read Huffman tree!" << std::endl;
        return;
    }

    std::cout << "TREECHECK" << std::endl;
    
    for (uint32_t i = 0u; i < nrVoxelTypes; ++i)
    {
        std::cout << "TYPE " << i << ": ";
        
        for (auto j = voxelTypeBits[i].cbegin(); j != voxelTypeBits[i].cend(); ++j)
        {
            std::cout << (*j ? "1" : "0");
        }
        
        std::cout << std::endl;
    }
    
    /*
    //Store tree.
    bits.insert(bits.end(), treeBits.begin(), treeBits.end());
    
    //Store run-length encoded voxels.
    const uint32_t nrBitsInFirstRun = 4u;
    int currentType = -1;
    int count = 0;
    
    std::cout << "STORING VOXELS" << std::endl;
    
    for (auto v = orderedVoxels.cbegin(); v != orderedVoxels.cend(); ++v)
    {
        if (*v != currentType || count >= 65535)
        {
            //Store counts up to 65535 voxels as 4-bits + 1-bit [+ 12-bits] where the latter 12 bits are only provided if 1-bit == 1.
            if (count > 0)
            {
                std::cout << count << " VOXELS OF TYPE " << currentType << std::endl;
                
                for (uint32_t i = 0u; i < nrBitsInFirstRun; ++i)
                {
                    bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
                }

                if (count >= (1 << nrBitsInFirstRun))
                {
                    bits.push_back(true);
                    
                    for (uint32_t i = nrBitsInFirstRun; i < 16u; ++i)
                    {
                        bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
                    }
                }
                else
                {
                    bits.push_back(false);
                }
            }
            
            //Add Huffman bits of current voxel type.
            currentType = *v;
            bits.insert(bits.end(), voxelTypeBits[currentType].begin(), voxelTypeBits[currentType].end());
            count = 1;
        }
        else
        {
            ++count;
        }
    }
    
    //Final run.
    std::cout << count << " FINAL VOXELS OF TYPE " << currentType << std::endl;
    
    for (uint32_t i = 0u; i < nrBitsInFirstRun; ++i)
    {
        bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
    }

    if (count >= (1 << nrBitsInFirstRun))
    {
        bits.push_back(true);
        
        for (uint32_t i = nrBitsInFirstRun; i < 16u; ++i)
        {
            bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
        }
    }
    else
    {
        bits.push_back(false);
    }
    */
    
    /*
    delete voxelTexture;
    
    voxelTexture = new draw::RGTexture3D(width, height, depth, draw::tf::none);
    
    voxelTexture->sendToDevice();
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    */
}

void GameVoxelMap::createVoxelPalette()
{
    //Create palette for different voxel types.
    const size_t n = voxelCubeArrayTexture->getDepth()/6;
    const size_t p = static_cast<size_t>(ceil(sqrtf(static_cast<float>(n))));
    size_t count = 0;
    
    for (size_t z = (voxelTexture->getDepth() - p)/2; z <= (voxelTexture->getDepth() + p)/2 && z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 1; y < 2; ++y)
        {
            for (size_t x = 0; x < p && x < voxelTexture->getWidth() && count < n; ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = 1 + count++;
            }
        }
    }
    
    voxelTexture->sendToDevice();
}

void GameVoxelMap::setVoxelBasePlane(const int &v)
{
    //Set all voxels at y = 0 to given type.
    for (size_t z = 0; z < voxelTexture->getDepth(); ++z)
    {
        for (size_t y = 0; y < 1; ++y)
        {
            for (size_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = v;
            }
        }
    }
    
    voxelTexture->sendToDevice();
}

void GameVoxelMap::setVoxel(const ivec3 &p, const int &v)
{
    (*voxelTexture)[voxelTexture->getChannels()*(clamp<int>(p.z + voxelTexture->getDepth()/2, 0, voxelTexture->getDepth() - 1)*voxelTexture->getHeight()*voxelTexture->getWidth() + clamp<int>(p.y, 0, voxelTexture->getHeight() - 1)*voxelTexture->getWidth() + clamp<int>(p.x + voxelTexture->getWidth()/2, 0, voxelTexture->getWidth() - 1)) + 0] = v;
    
    voxelTexture->sendToDevice();
}

int GameVoxelMap::getVoxel(const ivec3 &p) const
{
    return (*voxelTexture)[voxelTexture->getChannels()*(clamp<int>(p.z + voxelTexture->getDepth()/2, 0, voxelTexture->getDepth() - 1)*voxelTexture->getHeight()*voxelTexture->getWidth() + clamp<int>(p.y, 0, voxelTexture->getHeight() - 1)*voxelTexture->getWidth() + clamp<int>(p.x + voxelTexture->getWidth()/2, 0, voxelTexture->getWidth() - 1)) + 0];
}

float GameVoxelMap::getScale() const
{
    return voxelMap->getScale();
}

float GameVoxelMap::getScaledWidth() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getWidth());
}

float GameVoxelMap::getScaledHeight() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getHeight());
}

float GameVoxelMap::getScaledDepth() const
{
    return voxelMap->getScale()*static_cast<float>(voxelTexture->getDepth());
}

draw::VoxelIntersection GameVoxelMap::getIntersection(const vec3 &a_position, const vec3 &a_direction) const
{
    draw::VoxelIntersection intersection = voxelMap->getIntersection(*voxelTexture, a_position, a_direction);
    
    //Center voxel map.
    intersection.voxelIndices -= ivec3(voxelTexture->getWidth()/2, 0, voxelTexture->getDepth()/2);
    
    return intersection;
}

int GameVoxelMap::getBaseHeight(const int &x, const int &z) const
{
    //Determine height above voxel map.
    int baseHeight;
    
    for (baseHeight = 0; baseHeight < static_cast<int>(voxelTexture->getHeight()); ++baseHeight)
    {
        if ((*voxelTexture)(x + voxelTexture->getWidth()/2,
                            baseHeight,
                            z + voxelTexture->getDepth()/2).x == 0.0f)
        {
            break;
        }
    }
    
    return baseHeight;
}

std::string GameVoxelMap::getCompressedVoxels() const
{
    if (voxelTexture->getWidth() != voxelTexture->getHeight() ||
        voxelTexture->getWidth() != voxelTexture->getDepth() ||
        voxelTexture->getHeight() != voxelTexture->getDepth())
    {
        std::cerr << "Voxels can only be compressed for cubical voxel maps!" << std::endl;
        return "";
    }
    
    if ((voxelTexture->getWidth() & (voxelTexture->getWidth() - 1)) != 0 ||
        (voxelTexture->getHeight() & (voxelTexture->getHeight() - 1)) != 0 ||
        (voxelTexture->getDepth() & (voxelTexture->getDepth() - 1)) != 0)
    {
        std::cerr << "Voxels can only be compressed for power-of-two voxel maps!" << std::endl;
        return "";
    }
    
    //Order voxels via a space-filling curve to improve compression.
    const uint32_t nrVoxels = voxelTexture->getWidth()*voxelTexture->getHeight()*voxelTexture->getDepth();
    std::vector<uint8_t> orderedVoxels;
    
    //Use a Z-curve for each constant-Y plane to improve compression.
    for (uint32_t y = 0; y < voxelTexture->getHeight(); ++y)
    {
        /*
        for (uint32_t z = 0; z < voxelTexture->getDepth(); ++z)
        {
            for (uint32_t x = 0; x < voxelTexture->getWidth(); ++x)
            {
                orderedVoxels.push_back((*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0]);
            }
        }
        */
        
        for (uint32_t i = 0u; i < voxelTexture->getWidth()*voxelTexture->getDepth(); ++i)
        {
            uint32_t x = (i & 1u) | ((i & 4u) >> 1u) | ((i & 16u) >> 2u) | ((i & 64u) >> 3u) | ((i & 256u) >> 4u) | ((i & 1024u) >> 5u) | ((i & 4096u) >> 6u) | ((i & 16384u) >> 7u) | ((i & 65536u) >> 8u);
            uint32_t z =            ((i & 2u) >> 1u) | ((i & 8u) >> 2u) | ((i & 32u) >> 3u) | ((i & 128u) >> 4u) | ((i & 512u) >> 5u) | ((i & 2048u) >> 6u) | ((i & 8192u) >> 7u) | ((i & 32768u) >> 8u) | ((i & 131072u) >> 9u);
            
            orderedVoxels.push_back((*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0]);
        }
    }
    
    assert(orderedVoxels.size() == nrVoxels);
    
    //Determine the number of different voxel types and their frequencies.
    const uint32_t nrVoxelTypes = *std::max_element(orderedVoxels.begin(), orderedVoxels.end()) + 1u;
    std::vector<uint32_t> voxelTypeCounts(nrVoxelTypes, 0);
    
    for (auto i = orderedVoxels.cbegin(); i != orderedVoxels.cend(); ++i)
    {
        voxelTypeCounts[*i]++;
    }
    
    std::cout << "VOXEL COUNTS:" << std::endl;
    
    for (uint32_t i = 0; i < nrVoxelTypes; ++i)
    {
        std::cout << i << ": " << voxelTypeCounts[i] << std::endl;
    }
    
    //Create a Huffman tree to efficiently encode voxel types.
    std::multiset<HuffmanNode> preTree;
    std::vector<HuffmanNode> tree;
    
    for (uint32_t i = 0; i < nrVoxelTypes; ++i)
    {
        preTree.insert(HuffmanNode(i, voxelTypeCounts[i]));
    }
    
    while (preTree.size() >= 2)
    {
        //Grab two lowest-count nodes/leaves and remove them.
        auto np = preTree.begin();
        auto n1 = *np;
        np = preTree.erase(np);
        auto n2 = *np;
        preTree.erase(np);
        //Combine them into a node.
        auto n3 = HuffmanNode(n1, n2);
        
        std::cout << "PRETREE MERGE (" << preTree.size() << " left): (";
        for (auto i = n1.symbols.cbegin(); i != n1.symbols.cend(); ++i) std::cout << static_cast<int>(*i) << " ";
        std::cout << ", ";
        for (auto i = n2.symbols.cbegin(); i != n2.symbols.cend(); ++i) std::cout << static_cast<int>(*i) << " ";
        std::cout << ") --> ";
        for (auto i = n3.symbols.cbegin(); i != n3.symbols.cend(); ++i) std::cout << static_cast<int>(*i) << " ";
        std::cout << std::endl;
        
        n3.left = tree.size();
        n3.right = tree.size() + 1;
        
        //Add these to the final tree.
        if (n1.count <= n2.count)
        {
            tree.push_back(n1);
            tree.push_back(n2);
        }
        else
        {
            tree.push_back(n2);
            tree.push_back(n1);
        }
        
        //Add the combined node to the pre-tree to be processed.
        preTree.insert(n3);
    }
    
    //Should have exactly one node left (the root).
    assert(preTree.size() == 1);
    
    tree.push_back(*preTree.begin());

#ifndef NDEBUG
    //Check that leaves indeed have a single symbol.
    for (auto i = tree.cbegin(); i != tree.cend(); ++i)
    {
        assert((i->symbols.size() == 1 && i->left == -1 && i->right == -1) || (i->left >= 0 && i->left < static_cast<int>(tree.size()) && i->right >= 0 && i->right < static_cast<int>(tree.size())));
    }
#endif
    
    //Extract bits for each voxel type.
    std::vector<bool> treeBits;
    std::vector<std::vector<bool> > voxelTypeBits(nrVoxelTypes, std::vector<bool>());

    tree.back().treeToBits(tree, treeBits, voxelTypeBits);
    
    std::cout << "TREECHECK" << std::endl;
    
    for (uint32_t i = 0u; i < nrVoxelTypes; ++i)
    {
        std::cout << "TYPE " << i << ": ";
        
        for (auto j = voxelTypeBits[i].cbegin(); j != voxelTypeBits[i].cend(); ++j)
        {
            std::cout << (*j ? "1" : "0");
        }
        
        std::cout << std::endl;
    }
    
    //Generate output bits.
    std::vector<bool> bits;
    
    //Start with storing the voxel map size (should be a cube of power-of-two dimension).
    for (uint32_t i = 0u; i < 32u; ++i)
    {
        bits.push_back((voxelTexture->getWidth() & (1u << i)) != 0u);
    }
    
    //Store # of different voxel types.
    for (uint32_t i = 0u; i < 8u; ++i)
    {
        bits.push_back((nrVoxelTypes & (1u << i)) != 0u);
    }
    
    //Store tree.
    bits.insert(bits.end(), treeBits.begin(), treeBits.end());
    
    //Store run-length encoded voxels.
    const uint32_t nrBitsInFirstRun = 4u;
    int currentType = -1;
    int count = 0;
    
    std::cout << "STORING VOXELS" << std::endl;
    
    for (auto v = orderedVoxels.cbegin(); v != orderedVoxels.cend(); ++v)
    {
        if (*v != currentType || count >= 65535)
        {
            //Store counts up to 65535 voxels as 4-bits + 1-bit [+ 12-bits] where the latter 12 bits are only provided if 1-bit == 1.
            if (count > 0)
            {
                std::cout << count << " VOXELS OF TYPE " << currentType << std::endl;
                
                for (uint32_t i = 0u; i < nrBitsInFirstRun; ++i)
                {
                    bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
                }

                if (count >= (1 << nrBitsInFirstRun))
                {
                    bits.push_back(true);
                    
                    for (uint32_t i = nrBitsInFirstRun; i < 16u; ++i)
                    {
                        bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
                    }
                }
                else
                {
                    bits.push_back(false);
                }
            }
            
            //Add Huffman bits of current voxel type.
            currentType = *v;
            bits.insert(bits.end(), voxelTypeBits[currentType].begin(), voxelTypeBits[currentType].end());
            count = 1;
        }
        else
        {
            ++count;
        }
    }
    
    //Final run.
    std::cout << count << " FINAL VOXELS OF TYPE " << currentType << std::endl;
    
    for (uint32_t i = 0u; i < nrBitsInFirstRun; ++i)
    {
        bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
    }

    if (count >= (1 << nrBitsInFirstRun))
    {
        bits.push_back(true);
        
        for (uint32_t i = nrBitsInFirstRun; i < 16u; ++i)
        {
            bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
        }
    }
    else
    {
        bits.push_back(false);
    }
    
    std::cout << "MAPPING TO ASCII" << std::endl;
    
    //Map bits to ASCII for every 6-bit block.
    std::string text("");
    
    for (uint32_t i = 0; i < (bits.size()/6u) + 1; ++i)
    {
        uint32_t v = 0;
        
        for (uint32_t j = 0; j < 6u && (6u*i + j < bits.size()); ++j)
        {
            v |= (bits[6u*i + j] ? (1u << j) : 0u);
        }
        
        //Encode as ASCII safe for XML parsers.
        if (v < 26u)
        {
            text.push_back(v + 97u);
        }
        else if (v < 52u)
        {
            text.push_back(v + 65u - 26u);
        }
        else if (v < 62u)
        {
            text.push_back(v + 48u - 52u);
        }
        else
        {
            text.push_back(v + 42u - 62u);
        }
    }
    
    const size_t nrUncompressedBits = 8u*nrVoxels + 32u;
    
    std::cerr << "Compressed " << nrUncompressedBits << " bits to " << bits.size() << " bits encoded as " << 8u*text.size() << " ASCII bits (" << nrUncompressedBits/(8u*text.size()) << "x)." << std::endl;
    
    return text;
}


