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
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>

#include <tiny/img/io/image.h>

#include <tiny/draw/computetexture.h>
#include <tiny/draw/heightmap/scale.h>
#include <tiny/draw/heightmap/resize.h>
#include <tiny/draw/heightmap/normalmap.h>
#include <tiny/draw/heightmap/tangentmap.h>
#include <tiny/draw/heightmap/diamondsquare.h>

#include "terrain.h"

using namespace tanks;
using namespace tiny;

TanksTerrain::TanksTerrain(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading terrain resources..." << std::endl;
    
    assert(el);
    
    std::vector<img::Image> diffuseTextures;
    std::vector<img::Image> normalTextures;
    std::string heightMapFileName = "";
    std::string attributeShaderFileName = "";
    float widthScale = 1.0f;
    float heightScale = 1.0f;
    int farScale = 2; 
    float detailScale = 1.0f;
    
    assert(el->ValueStr() == "terrain");
    
    el->QueryFloatAttribute("scale_width", &widthScale);
    el->QueryFloatAttribute("scale_height", &heightScale);
    el->QueryIntAttribute("scale_far", &farScale);
    el->QueryFloatAttribute("scale_detail", &detailScale);
    el->QueryStringAttribute("attribute_shader", &attributeShaderFileName);
    el->QueryStringAttribute("heightmap", &heightMapFileName);
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "biome")
        {
            if (sl->Attribute("diffuse")) diffuseTextures.push_back(img::io::readImage(path + std::string(sl->Attribute("diffuse"))));
            if (sl->Attribute("normal")) normalTextures.push_back(img::io::readImage(path + std::string(sl->Attribute("normal"))));
        }
    }
    
    //Read attribute shader.
    if (true)
    {
        std::ifstream file((path + attributeShaderFileName).c_str());
        
        if (!file.good())
        {
            std::cerr << "Unable to open attribute shader file '" << attributeShaderFileName << "'!" << std::endl;
            throw std::exception();
        }
        
        terrainAttributeShader = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        file.close();
    }
    
    //Actually construct the terrain using all the data that we have.
    terrainScale = vec2(widthScale);
    terrainFarScale = ivec2(farScale);
    terrainDetailScale = vec2(detailScale);
    
    //Create height maps.
    terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(path + heightMapFileName), draw::tf::filter);
    terrainFarHeightTexture = new draw::FloatTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight(), draw::tf::filter);
    
    //Create normal maps for the far-away and zoomed-in heightmaps.
    terrainFarNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainFarTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    terrainTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
    
    //Create an attribute texture that determines the terrain type (forest/grass/mud/stone) based on the altitude and slope.
    terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth(), 0, 0, 0, 0));
    terrainFarAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
    
    //Read diffuse textures and make them tileable.
    biomeDiffuseTextures = new draw::RGBTexture2DArray(diffuseTextures.begin(), diffuseTextures.end());
    biomeNormalTextures = new draw::RGBTexture2DArray(normalTextures.begin(), normalTextures.end());
    
    //Paint the terrain with the zoomed-in and far-away textures.
    terrain = new draw::Terrain(6, 8);
    
    //Scale vertical range of the far-away heightmap.
    draw::computeScaledTexture(*terrainHeightTexture, *terrainFarHeightTexture, vec4(heightScale/255.0f), vec4(0.0f));
    
    //Zoom in on part of the terrain.
    setOffset(vec2(0.5f));
}

TanksTerrain::~TanksTerrain()
{
    delete terrain;
    delete terrainHeightTexture;
    delete terrainFarHeightTexture;
    delete terrainNormalTexture;
    delete terrainFarNormalTexture;
    delete terrainTangentTexture;
    delete terrainFarTangentTexture;
    delete terrainAttributeTexture;
    delete terrainFarAttributeTexture;
    delete biomeDiffuseTextures;
    delete biomeNormalTextures;
}

void TanksTerrain::calculateAttributes(const tiny::draw::FloatTexture2D &heightMap, tiny::draw::RGBATexture2D &attributeMap, const float &scale) const
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");

    draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, outputTextures, terrainAttributeShader);
    
    computeTexture->uniformMap().setFloatUniform(2.0f*scale, "mapScale");
    computeTexture->setInput(heightMap, "source");
    computeTexture->setOutput(attributeMap, "colour");
    computeTexture->compute();
    attributeMap.getFromDevice();
    
    delete computeTexture;
}

void TanksTerrain::setOffset(const vec2 &offset)
{
    //Shifts the local heightmap to a specific spot in the global heightmap.
    terrainFarOffset = offset;
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*terrainFarHeightTexture, *terrainHeightTexture,
                                vec2(1.0f/static_cast<float>(terrainFarScale.x), 1.0f/static_cast<float>(terrainFarScale.y)),
                                offset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*terrainHeightTexture, *terrainHeightTexture, terrainFarScale.x);
    
    //Calculate normal and tangent maps.
    draw::computeNormalMap(*terrainFarHeightTexture, *terrainFarNormalTexture, terrainScale.x*terrainFarScale.x);
    draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
    draw::computeTangentMap(*terrainFarHeightTexture, *terrainFarTangentTexture, terrainScale.x*terrainFarScale.x);
    draw::computeTangentMap(*terrainHeightTexture, *terrainTangentTexture, terrainScale.x);
    
    //Calculate attribute maps for both the zoomed-in and far-away terrain.
    calculateAttributes(*terrainHeightTexture, *terrainAttributeTexture, terrainScale.x);
    calculateAttributes(*terrainFarHeightTexture, *terrainFarAttributeTexture, terrainScale.x*terrainFarScale.x);
    
    //Set proper textures in the terrain.
    terrain->setFarHeightTextures(*terrainHeightTexture, *terrainFarHeightTexture,
                                  *terrainTangentTexture, *terrainFarTangentTexture,
                                  *terrainNormalTexture, *terrainFarNormalTexture,
                                  terrainScale, terrainFarScale, terrainFarOffset);
    terrain->setFarDiffuseTextures(*terrainAttributeTexture, *terrainFarAttributeTexture,
                                   *biomeDiffuseTextures, *biomeNormalTextures,
                                   terrainDetailScale);
}

float TanksTerrain::getHeight(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*terrainHeightTexture, terrainScale, a_pos).x;
}

vec4 TanksTerrain::getAttributes(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*terrainAttributeTexture, terrainScale, a_pos);
}

