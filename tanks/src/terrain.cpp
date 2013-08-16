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
    
    std::vector<img::Image> diffuseImages;
    std::vector<img::Image> normalImages;
    std::string heightMapFileName = "";
    std::string attributeShaderFileName = "";
    float widthScaleFactor = 1.0f;
    float heightScaleFactor = 1.0f;
    int farScaleFactor = 2; 
    float detailScaleFactor = 1.0f;
    
    assert(el->ValueStr() == "terrain");
    
    el->QueryFloatAttribute("scale_width", &widthScaleFactor);
    el->QueryFloatAttribute("scale_height", &heightScaleFactor);
    el->QueryIntAttribute("scale_far", &farScaleFactor);
    el->QueryFloatAttribute("scale_detail", &detailScaleFactor);
    el->QueryStringAttribute("attribute_shader", &attributeShaderFileName);
    el->QueryStringAttribute("heightmap", &heightMapFileName);
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "biome")
        {
            if (sl->Attribute("diffuse")) diffuseImages.push_back(img::io::readImage(path + std::string(sl->Attribute("diffuse"))));
            if (sl->Attribute("normal")) normalImages.push_back(img::io::readImage(path + std::string(sl->Attribute("normal"))));
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
        
        attributeShaderCode = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        file.close();
    }
    
    //Actually construct the terrain using all the data that we have.
    scale = vec2(widthScaleFactor);
    farScale = ivec2(farScaleFactor);
    farOffset = vec2(0.5f);
    localTextureScale = vec2(detailScaleFactor);
    
    //Create height maps.
    heightTexture = new draw::FloatTexture2D(img::io::readImage(path + heightMapFileName), draw::tf::filter);
    farHeightTexture = new draw::FloatTexture2D(heightTexture->getWidth(), heightTexture->getHeight(), draw::tf::filter);
    
    //Create normal maps for the far-away and zoomed-in heightmaps.
    farTangentTexture = new draw::RGBTexture2D(heightTexture->getWidth(), heightTexture->getHeight());
    tangentTexture = new draw::RGBTexture2D(heightTexture->getWidth(), heightTexture->getHeight());
    farNormalTexture = new draw::RGBTexture2D(heightTexture->getWidth(), heightTexture->getHeight());
    normalTexture = new draw::RGBTexture2D(heightTexture->getWidth(), heightTexture->getHeight());
    
    //Create an attribute texture that determines the terrain type (forest/grass/mud/stone) based on the altitude and slope.
    attributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(heightTexture->getWidth()));
    farAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(heightTexture->getWidth()));
    
    //Create local diffuse textures.
    if (diffuseImages.empty()) localDiffuseTextures = new draw::RGBTexture2DArray(img::Image::createSolidImage());
    else localDiffuseTextures = new draw::RGBTexture2DArray(diffuseImages.begin(), diffuseImages.end());
    
    if (normalImages.empty()) localNormalTextures = new draw::RGBTexture2DArray(img::Image::createUpNormalImage());
    else localNormalTextures = new draw::RGBTexture2DArray(normalImages.begin(), normalImages.end());
    
    //Scale vertical range of the far-away heightmap.
    draw::computeScaledTexture(*heightTexture, *farHeightTexture, vec4(heightScaleFactor/255.0f), vec4(0.0f));
    
    //Create the terrain.
    terrain = new draw::Terrain(6, 8);
    setOffset(vec2(0.5f));
}

TanksTerrain::~TanksTerrain()
{
    delete terrain;
    delete heightTexture;
    delete farHeightTexture;
    delete tangentTexture;
    delete farTangentTexture;
    delete normalTexture;
    delete farNormalTexture;
    delete attributeTexture;
    delete farAttributeTexture;
    delete localDiffuseTextures;
    delete localNormalTextures;
}

void TanksTerrain::setOffset(const vec2 &offset)
{
    farOffset = offset;
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*farHeightTexture, *heightTexture,
                                vec2(1.0f/static_cast<float>(farScale.x), 1.0f/static_cast<float>(farScale.y)),
                                farOffset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*heightTexture, *heightTexture, farScale.x);
    
    //Tangent and normal maps.
    draw::computeTangentMap(*farHeightTexture, *farTangentTexture, scale.x*farScale.x);
    draw::computeTangentMap(*heightTexture, *tangentTexture, scale.x);
    draw::computeNormalMap(*farHeightTexture, *farNormalTexture, scale.x*farScale.x);
    draw::computeNormalMap(*heightTexture, *normalTexture, scale.x);
    
    //Calculate attribute maps for both the zoomed-in and far-away terrain.
    calculateAttributes(*heightTexture, *attributeTexture, attributeShaderCode, scale.x);
    calculateAttributes(*farHeightTexture, *farAttributeTexture, attributeShaderCode, scale.x*farScale.x);
    terrain->setFarDiffuseTextures(*attributeTexture, *farAttributeTexture, *localDiffuseTextures, *localNormalTextures, localTextureScale);
    //terrain->setDiffuseTextures(*attributeTexture, *localDiffuseTextures, *localNormalTextures, vec2(1.0f, 1.0f));
    
    //Paint the terrain with the zoomed-in and far-away textures.
    terrain->setFarHeightTextures(*heightTexture, *farHeightTexture,
                                  *tangentTexture, *farTangentTexture,
                                  *normalTexture, *farNormalTexture,
                                  scale, farScale, farOffset);
    //terrain->setHeightTextures(*farHeightTexture, *farTangentTexture, *farNormalTexture, scale);
    //terrain->setHeightTextures(*heightTexture, *tangentTexture, *normalTexture, scale);
}

void TanksTerrain::calculateAttributes(const tiny::draw::FloatTexture2D &heightMap, tiny::draw::RGBATexture2D &attributeMap, const std::string &shaderCode, const float &scale)
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");

    draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, outputTextures, shaderCode);
    
    computeTexture->uniformMap().setFloatUniform(2.0f*scale, "mapScale");
    computeTexture->setInput(heightMap, "source");
    computeTexture->setOutput(attributeMap, "colour");
    computeTexture->compute();
    attributeMap.getFromDevice();
    
    delete computeTexture;
}

float TanksTerrain::getHeight(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*heightTexture, scale, a_pos).x;
}

vec4 TanksTerrain::getAttributes(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*attributeTexture, scale, a_pos);
}

