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
#include <exception>

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
    
    std::set<uint8_t> symbols;
    std::vector<bool> bits;
    uint32_t count;
    int left, right;
};

//Following https://stackoverflow.com/questions/759707/efficient-way-of-storing-huffman-tree.
void huffmanTreeToBits(std::vector<HuffmanNode> &tree, const int &index, std::vector<bool> &treeBits, std::vector<std::vector<bool> > &symbolBits)
{
    const HuffmanNode n = tree[index];
    
    if (n.leaf())
    {
        //Leaf node.
        treeBits.push_back(true);
        
        const uint8_t v = *(n.symbols.begin());
        
        for (uint32_t i = 0u; i < 8u; ++i)
        {
            treeBits.push_back((v & (1u << i)) != 0u);
        }
        
        symbolBits[v] = n.bits;
    }
    else
    {
        //Non-leaf node.
        treeBits.push_back(false);
        
        tree[n.left].bits = n.bits;
        tree[n.left].bits.push_back(false);
        huffmanTreeToBits(tree, n.left, treeBits, symbolBits);
        tree[n.right].bits = n.bits;
        tree[n.right].bits.push_back(true);
        huffmanTreeToBits(tree, n.right, treeBits, symbolBits);
    }
}

size_t huffmanBitsToTree(const std::vector<bool> &treeBits, size_t bp, std::vector<HuffmanNode> &tree, const int &index, std::vector<std::vector<bool> > &symbolBits)
{
    HuffmanNode n = tree[index];
    
    if (bp >= treeBits.size())
    {
        std::cerr << "Unable to decode Huffman tree bits!" << std::endl;
        return treeBits.size();
    }
    
    if (treeBits[bp++])
    {
        //Leaf node.
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
        
        n.symbols.clear();
        n.symbols.insert(v);
        symbolBits[v] = n.bits;
    }
    else
    {
        //Non-leaf node.
        n.left = tree.size();
        tree.push_back(HuffmanNode());
        tree[n.left].bits = n.bits;
        tree[n.left].bits.push_back(false);
        bp = huffmanBitsToTree(treeBits, bp, tree, n.left, symbolBits);
        
        n.right = tree.size();
        tree.push_back(HuffmanNode());
        tree[n.right].bits = n.bits;
        tree[n.right].bits.push_back(true);
        bp = huffmanBitsToTree(treeBits, bp, tree, n.right, symbolBits);
    }
    
    tree[index] = n;
    
    return bp;
}

void uintToBits(std::vector<bool> &bits, const uint32_t &value, const uint32_t &nrBits)
{
    for (uint32_t i = 0u; i < nrBits; ++i)
    {
        bits.push_back((value & (1u << i)) != 0u);
    }
}

uint32_t bitsToUint(const std::vector<bool> &bits, size_t &bp, const uint32_t &nrBits)
{
    uint32_t v = 0u;
    
    for (uint32_t i = 0u; i < nrBits; ++i)
    {
        v |= (bits[bp++] ? (1u << i) : 0u);
    }
    
    return v;
}

void voxelRunCountToBits(std::vector<bool> &bits, const uint32_t &count, const uint32_t &medNrCountBits, const uint32_t &maxNrCountBits)
{
    //Only a single voxel?
    for (uint32_t i = 0u; i < 1u; ++i)
    {
        bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
    }

    if (count >= 2u)
    {
        //Only voxels up to the short run length?
        bits.push_back(true);
        
        for (uint32_t i = 1u; i < medNrCountBits; ++i)
        {
            bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
        }

        if (count >= (1u << medNrCountBits))
        {
            //Up to full run length.
            bits.push_back(true);
            
            for (uint32_t i = medNrCountBits; i < maxNrCountBits; ++i)
            {
                bits.push_back((static_cast<uint32_t>(count) & (1u << i)) != 0u);
            }
        }
        else
        {
            bits.push_back(false);
        }
    }
    else
    {
        bits.push_back(false);
    }
}

uint32_t bitsToVoxelRunCount(std::vector<bool> &bits, size_t &bp, const uint32_t &medNrCountBits, const uint32_t &maxNrCountBits)
{
    uint32_t count = 0u;
    
    for (uint32_t i = 0u; i < 1u; ++i)
    {
        count |= (bits[bp++] ? (1u << i) : 0u);
    }
    
    if (bits[bp++])
    {
        for (uint32_t i = 1u; i < medNrCountBits; ++i)
        {
            count |= (bits[bp++] ? (1u << i) : 0u);
        }
        
        if (bits[bp++])
        {
            for (uint32_t i = medNrCountBits; i < maxNrCountBits; ++i)
            {
                count |= (bits[bp++] ? (1u << i) : 0u);
            }
        }
    }
    
    return count;
}

draw::RGTexture3D *initializeVoxelTexture(const size_t &width, const size_t &height, const size_t &depth)
{
    //Initialize checkerboard voxel map.
    draw::RGTexture3D *voxelTexture = new draw::RGTexture3D(width, height, depth, draw::tf::none);
    
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
    
    return voxelTexture;
}

GameVoxelMap::GameVoxelMap(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading voxel map resources..." << std::endl;
    
    assert(std::string(el->Value()) == "voxelmap");
    
    int width = 64;
    int height = 64;
    int depth = 64;
    float voxelSize = 1.0f;
    std::string text = "";
    
    el->QueryIntAttribute("width", &width);
    el->QueryIntAttribute("height", &height);
    el->QueryIntAttribute("depth", &depth);
    el->QueryFloatAttribute("scale", &voxelSize);
    el->QueryStringAttribute("compressed_voxels", &text);
    
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
    voxelMap = new draw::VoxelMap(std::max(width, std::max(height, depth))*3);
    
    if (text.empty())
    {
        voxelTexture = initializeVoxelTexture(width, height, depth);
        setVoxelBasePlane(1);
        createVoxelPalette();
    }
    else
    {
        if (!createFromCompressedVoxels(text, voxelSize))
        {
            std::cerr << "Unable to initialize voxel map from compressed data '" << text << "'!" << std::endl;
            throw std::exception();
        }
    }
    
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    voxelMap->setCubeMaps(*voxelCubeArrayTexture);
}

GameVoxelMap::~GameVoxelMap()
{
    delete voxelMap;
    delete voxelTexture;
    delete voxelCubeArrayTexture;
}

bool GameVoxelMap::createFromCompressedVoxels(const std::string &text, const float &voxelSize)
{
    //Create voxels from output of getCompressedVoxels().
    
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
            return false;
        }
        
        assert(v < 64u);
        
        for (uint32_t j = 0; j < 6u; ++j)
        {
            bits.push_back((v & (1u << j)) != 0u);
        }
    }
    
    size_t bp = 0;
    
    const uint32_t width = bitsToUint(bits, bp, 16u);
    const uint32_t height = bitsToUint(bits, bp, 16u);
    const uint32_t depth = bitsToUint(bits, bp, 16u);
    const uint32_t medNrCountBits = bitsToUint(bits, bp, 6u);
    const uint32_t maxNrCountBits = bitsToUint(bits, bp, 6u);
    const uint32_t nrVoxelTypes = bitsToUint(bits, bp, 8u);
    
    if (width != depth ||
        ((width - 1u) & width) != 0u ||
        ((depth - 1u) & depth) != 0u)
    {
        std::cerr << "Invalid dimensions " << width << "x" << height << "x" << depth << " for the voxel map (require equal width and depth, as power of two)!" << std::endl;
        return false;
    }
    
    if (medNrCountBits >= maxNrCountBits)
    {
        std::cerr << "Invalid bit counts (" << medNrCountBits << ", " << maxNrCountBits << ")!" << std::endl;
        return false;
    }
    
    std::cerr << "Generating a " << width << "x" << height << "x" << depth << " voxel map containing " << nrVoxelTypes << " voxel types from " << text.size() << " ASCII bytes of compressed data..." << std::endl;

    std::vector<HuffmanNode> tree;
    std::vector<std::vector<bool> > voxelTypeBits(nrVoxelTypes, std::vector<bool>());
    
    tree.push_back(HuffmanNode());
    bp = huffmanBitsToTree(bits, bp, tree, 0, voxelTypeBits);
    
    if (bp >= bits.size() || voxelTypeBits.size() != nrVoxelTypes)
    {
        std::cerr << "Unable to read Huffman tree!" << std::endl;
        return false;
    }

    const uint32_t nrVoxels = width*width*width;
    std::vector<uint8_t> orderedVoxels;
    
    while (orderedVoxels.size() < nrVoxels && bp < bits.size())
    {
        //Read voxel type using the Huffman tree.
        HuffmanNode n = tree.front();
        
        while (!n.leaf())
        {
            if (bits[bp++]) n = tree[n.right];
            else n = tree[n.left];
        }
        
        //Store the found symbols the read count # times.
        orderedVoxels.insert(orderedVoxels.end(), bitsToVoxelRunCount(bits, bp, medNrCountBits, maxNrCountBits), *(n.symbols.begin()));
    }
    
    if (nrVoxels != orderedVoxels.size())
    {
        std::cerr << "Unable to decode voxels from bits!" << std::endl;
        return false;
    }
    
    //Fill in voxel texture.
    if (voxelTexture) delete voxelTexture;
    
    voxelTexture = initializeVoxelTexture(width, height, depth);
    
    //Use a Z-curve for each constant-Y plane to improve compression.
    for (uint32_t y = 0; y < voxelTexture->getHeight(); ++y)
    {
        for (uint32_t i = 0u; i < voxelTexture->getWidth()*voxelTexture->getDepth(); ++i)
        {
            uint32_t x = (i & 1u) | ((i & 4u) >> 1u) | ((i & 16u) >> 2u) | ((i & 64u) >> 3u) | ((i & 256u) >> 4u) | ((i & 1024u) >> 5u) | ((i & 4096u) >> 6u) | ((i & 16384u) >> 7u) | ((i & 65536u) >> 8u);
            uint32_t z =            ((i & 2u) >> 1u) | ((i & 8u) >> 2u) | ((i & 32u) >> 3u) | ((i & 128u) >> 4u) | ((i & 512u) >> 5u) | ((i & 2048u) >> 6u) | ((i & 8192u) >> 7u) | ((i & 32768u) >> 8u) | ((i & 131072u) >> 9u);
            
            (*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0] = orderedVoxels[width*width*y + i];
        }
    }
    
    voxelTexture->sendToDevice();
    voxelMap->setVoxelMap(*voxelTexture, voxelSize);
    
    std::cerr << "Created voxel map from compressed data." << std::endl;
    
    return true;
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
    if (voxelTexture->getWidth() != voxelTexture->getDepth())
    {
        std::cerr << "Voxels can only be compressed for voxel maps of equal width and depth!" << std::endl;
        return "";
    }
    
    if ((voxelTexture->getWidth() & (voxelTexture->getWidth() - 1)) != 0 ||
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
        for (uint32_t i = 0u; i < voxelTexture->getWidth()*voxelTexture->getDepth(); ++i)
        {
            uint32_t x = (i & 1u) | ((i & 4u) >> 1u) | ((i & 16u) >> 2u) | ((i & 64u) >> 3u) | ((i & 256u) >> 4u) | ((i & 1024u) >> 5u) | ((i & 4096u) >> 6u) | ((i & 16384u) >> 7u) | ((i & 65536u) >> 8u);
            uint32_t z =            ((i & 2u) >> 1u) | ((i & 8u) >> 2u) | ((i & 32u) >> 3u) | ((i & 128u) >> 4u) | ((i & 512u) >> 5u) | ((i & 2048u) >> 6u) | ((i & 8192u) >> 7u) | ((i & 32768u) >> 8u) | ((i & 131072u) >> 9u);
            
            orderedVoxels.push_back((*voxelTexture)[voxelTexture->getChannels()*(z*voxelTexture->getHeight()*voxelTexture->getWidth() + y*voxelTexture->getWidth() + x) + 0]);
        }
    }
    
    assert(orderedVoxels.size() == nrVoxels);
    
    //Determine the number of different voxel types and their frequencies in a run-length compression scheme.
    const uint32_t maxNrCountBits = 16u;
    const uint32_t nrVoxelTypes = *std::max_element(orderedVoxels.begin(), orderedVoxels.end()) + 1u;
    std::vector<uint32_t> voxelTypeCounts(nrVoxelTypes, 0);
    int currentType = -1;
    int count = 0;
    std::vector<int> countValues;
    
    for (auto v = orderedVoxels.cbegin(); v != orderedVoxels.cend(); ++v)
    {
        if (*v != currentType || count >= ((1 << maxNrCountBits) - 1))
        {
            if (count > 0) countValues.push_back(count);
            voxelTypeCounts[*v]++;
            currentType = *v;
            count = 1;
        }
        else
        {
            ++count;
        }
    }
    
    //Find good cutoffs for voxel counts for this specific map by taking 90-th percentile of run lengths.
    std::sort(countValues.begin(), countValues.end());
    
    uint32_t medNrCountBits;
    const uint32_t shortRunCount = countValues[static_cast<int>(floor(0.9f*static_cast<float>(countValues.size())))];
    
    for (medNrCountBits = 0u; medNrCountBits < maxNrCountBits; ++medNrCountBits)
    {
        if ((1u << medNrCountBits) >= shortRunCount) break;
    }
    
    //Create a Huffman tree to efficiently encode voxel types.
    std::multiset<HuffmanNode> preTree;
    std::vector<HuffmanNode> tree;
    
    for (uint32_t i = 0u; i < nrVoxelTypes; ++i)
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

    huffmanTreeToBits(tree, tree.size() - 1, treeBits, voxelTypeBits);
    
    //Generate output bits.
    std::vector<bool> bits;
    
    //Start with storing the voxel map size.
    uintToBits(bits, voxelTexture->getWidth(), 16u);
    uintToBits(bits, voxelTexture->getHeight(), 16u);
    uintToBits(bits, voxelTexture->getDepth(), 16u);
    
    //Store run # bits.
    uintToBits(bits, medNrCountBits, 6u);
    uintToBits(bits, maxNrCountBits, 6u);
    
    //Store # of different voxel types.
    uintToBits(bits, nrVoxelTypes, 8u);
    
    //Store tree.
    bits.insert(bits.end(), treeBits.begin(), treeBits.end());
    
    //Store run-length encoded voxels.
    currentType = -1;
    count = 0;
    
    for (auto v = orderedVoxels.cbegin(); v != orderedVoxels.cend(); ++v)
    {
        if (*v != currentType || count >= ((1 << maxNrCountBits) - 1))
        {
            if (count > 0)
            {
                voxelRunCountToBits(bits, count, medNrCountBits, maxNrCountBits);
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
    voxelRunCountToBits(bits, count, medNrCountBits, maxNrCountBits);
    
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
    
    std::cerr << "Compressed " << voxelTexture->getWidth() << "x" << voxelTexture->getHeight() << "x" << voxelTexture->getDepth() << " voxel map " << nrUncompressedBits << " bits to " << bits.size() << " bits encoded as " << 8u*text.size() << " ASCII bits (" << nrUncompressedBits/(8u*text.size()) << "x). Run lengths stored in 1, " << medNrCountBits << ", and " << maxNrCountBits << " bits." << std::endl;
    
    return text;
}


