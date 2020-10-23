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

using namespace moba;
using namespace tiny;

GameTerrain::GameTerrain(const std::string &path, TiXmlElement *el)
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
    float offset_x = 0.5f;
    float offset_y = 0.5f;
    
    assert(el->ValueStr() == "terrain");
    
    el->QueryFloatAttribute("scale_width", &widthScaleFactor);
    el->QueryFloatAttribute("scale_height", &heightScaleFactor);
    el->QueryIntAttribute("scale_far", &farScaleFactor);
    el->QueryFloatAttribute("scale_detail", &detailScaleFactor);
    el->QueryStringAttribute("attribute_shader", &attributeShaderFileName);
    el->QueryStringAttribute("heightmap", &heightMapFileName);
    el->QueryFloatAttribute("offset_x", &offset_x);
    el->QueryFloatAttribute("offset_y", &offset_y);
    
    for (TiXmlElement *sl = el->FirstChildElement(); sl; sl = sl->NextSiblingElement())
    {
        if (sl->ValueStr() == "biome")
        {
            if (sl->Attribute("diffuse")) diffuseImages.push_back(img::io::readImage(path + std::string(sl->Attribute("diffuse"))));
            else diffuseImages.push_back(img::Image::createTestImage());
            if (sl->Attribute("normal")) normalImages.push_back(img::io::readImage(path + std::string(sl->Attribute("normal"))));
            else normalImages.push_back(img::Image::createUpNormalImage(diffuseImages.back().width));
            //if (sl->Attribute("sound")) biomeSounds.push_back(snd::io::readSample(path + std::string(sl->Attribute("sound"))));
            //else missData = true;
        }
        else if (sl->ValueStr() == "attributes")
        {
            int biomeIndex = -1;
            std::string attributeMapFileName = "";
            
            sl->QueryIntAttribute("biome_index", &biomeIndex);
            sl->QueryStringAttribute("map", &attributeMapFileName);
            
            if (biomeIndex < 0 || biomeIndex >= static_cast<int>(diffuseImages.size()))
            {
                std::cerr << "Invalid biome index for attribute map!" << std::endl;
                throw std::exception();
            }
            
            attributeMaps.push_back(std::make_pair(biomeIndex, img::io::readImage(path + attributeMapFileName)));
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
    setOffset(vec2(offset_x - 0.5f/static_cast<float>(farScale.x), offset_y - 0.5f/static_cast<float>(farScale.y)));
}

GameTerrain::~GameTerrain()
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

void GameTerrain::setOffset(const vec2 &offset)
{
    farOffset = offset;
    
    //Zoom into a small area of the far-away heightmap.
    draw::computeResizedTexture(*farHeightTexture, *heightTexture,
                                vec2(1.0f/static_cast<float>(farScale.x), 1.0f/static_cast<float>(farScale.y)),
                                farOffset);
    
    //Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
    draw::computeDiamondSquareRefinement(*heightTexture, *heightTexture, farScale.x, 0.5f);
    
    //Tangent and normal maps.
    draw::computeTangentMap(*farHeightTexture, *farTangentTexture, scale.x*farScale.x);
    draw::computeTangentMap(*heightTexture, *tangentTexture, scale.x);
    draw::computeNormalMap(*farHeightTexture, *farNormalTexture, scale.x*farScale.x);
    draw::computeNormalMap(*heightTexture, *normalTexture, scale.x);
    
    //Calculate attribute maps for both the zoomed-in and far-away terrain.
    calculateAttributes(*heightTexture, *attributeTexture, attributeShaderCode, scale.x);
    calculateAttributes(*farHeightTexture, *farAttributeTexture, attributeShaderCode, scale.x*farScale.x);
    
    //Blend in user-supplied attribute maps.
    for (std::list<std::pair<int, img::Image> >::const_iterator i = attributeMaps.begin(); i != attributeMaps.end(); ++i)
    {
        //Scale attribute map using the terrain scaling.
        draw::RGBATexture2D *tex1 = new draw::RGBATexture2D(i->second);
        draw::RGBATexture2D *tex2 = new draw::RGBATexture2D(i->second);

        draw::computeResizedTexture(*tex1, *tex2,
                                    vec2(1.0f/static_cast<float>(farScale.x), 1.0f/static_cast<float>(farScale.y)),
                                    farOffset);
        applyUserAttributeMap(*attributeTexture, i->first, *tex2);
        applyUserAttributeMap(*farAttributeTexture, i->first, *tex1);
        
        delete tex1;
        delete tex2;
    }
    
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

vec3 GameTerrain::getWorldPosition(const vec2 &p) const
{
    //Convert position within texture (in [0, 1] x [0, 1]) to XYZ world position.
    vec2 pos = vec2(p.x - (farOffset.x + 0.5f/static_cast<float>(farScale.x)), p.y - (farOffset.y + 0.5f/static_cast<float>(farScale.y)));
    
    pos.x *= static_cast<float>(heightTexture->getWidth()*farScale.x)*scale.x;
    pos.y *= static_cast<float>(heightTexture->getHeight()*farScale.y)*scale.y;

    return vec3(pos.x, GameTerrain::sampleTextureBilinear(*heightTexture, scale, pos).x, pos.y);
}

//Function to generate position samples of a certain attribute map.
int GameTerrain::createAttributeMapSamples(const int &maxNrSamples, const int &index,
                                           std::vector<draw::StaticMeshInstance> &highDetailInstances, const vec2 &lowDetailInstanceSize, std::vector<draw::WorldIconInstance> &lowDetailInstances, std::vector<vec3> &positions) const
{
    const float maxDistance = 0.5f*heightTexture->getWidth()*scale.x;
    int nrSamples = 0;
    
    highDetailInstances.clear();
    lowDetailInstances.clear();
    positions.clear();
    
    if (maxNrSamples <= 0)
    {
        std::cerr << "Warning: Not placing any samples!" << std::endl;
        return 0;
    }

    std::cerr << "Placing up to " << maxNrSamples << " samples..." << std::endl;
    
    highDetailInstances.reserve(maxNrSamples);
    lowDetailInstances.reserve(maxNrSamples);
    positions.reserve(maxNrSamples);
    
    while (nrSamples < maxNrSamples)
    {
        //Determine the random spot where we want to place a sample.
        const vec2 samplePlanePosition = randomVec2(maxDistance);
        
        //Are we going to place a sample here?
        const float placeProbability = 255.0f*GameTerrain::sampleTextureBilinear(*attributeTexture, scale, samplePlanePosition).x - static_cast<float>(index);
        
        if (placeProbability <= 0.5f && placeProbability >= -0.5f)
        {
            //Determine height.
            const vec3 samplePosition = vec3(samplePlanePosition.x, GameTerrain::sampleTextureBilinear(*heightTexture, scale, samplePlanePosition).x, samplePlanePosition.y);
            
            highDetailInstances.push_back(draw::StaticMeshInstance(vec4(samplePosition.x, samplePosition.y, samplePosition.z, 1.0f),
                                                                   vec4(0.0f, 0.0f, 0.0f, 1.0f)));
            lowDetailInstances.push_back(draw::WorldIconInstance(vec3(samplePosition.x, samplePosition.y + lowDetailInstanceSize.y, samplePosition.z),
                                                                 lowDetailInstanceSize,
                                                                 vec4(0.0f, 0.0f, 1.0f, 1.0f),
                                                                 vec4(1.0f, 1.0f, 1.0f, 1.0f)));
            positions.push_back(samplePosition);
            ++nrSamples;
        }
    }

    std::cerr << "Placed " << nrSamples << " samples." << std::endl;
    
    return nrSamples;
}

void GameTerrain::applyUserAttributeMap(draw::RGBATexture2D &attributeMap, const int &biomeIndex, const draw::RGBATexture2D &userMap)
{
    std::cerr << "Applying user attribute map for biome " << biomeIndex << "..." << std::endl;
    
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    
    inputTextures.push_back("source1");
    inputTextures.push_back("source2");
    outputTextures.push_back("colour");

    draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, attributeMap.getWidth(), attributeMap.getHeight(), outputTextures,
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source1;\n"
"uniform sampler2D source2;\n"
"uniform float biomeIndex;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"    vec4 s1 = texture(source1, tex);\n"
"    vec4 s2 = texture(source2, tex);\n"
"   \n"
"    colour = mix(s1, vec4(biomeIndex, 0.0f, 0.0f, 0.0f), s2.w);\n"
"}\n");
    
    computeTexture->uniformMap().setFloatUniform(static_cast<float>(biomeIndex)/255.0f, "biomeIndex");
    computeTexture->setInput(attributeMap, "source1");
    computeTexture->setInput(userMap, "source2");
    computeTexture->setOutput(attributeMap, "colour");
    computeTexture->compute();
    attributeMap.getFromDevice();
    
    delete computeTexture;
}

void GameTerrain::calculateAttributes(const tiny::draw::FloatTexture2D &heightMap, tiny::draw::RGBATexture2D &attributeMap, const std::string &shaderCode, const float &scale)
{
    std::vector<std::string> inputTextures;
    std::vector<std::string> outputTextures;
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");

    draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, attributeMap.getWidth(), attributeMap.getHeight(), outputTextures, shaderCode);
    
    computeTexture->uniformMap().setFloatUniform(2.0f*scale, "mapScale");
    computeTexture->setInput(heightMap, "source");
    computeTexture->setOutput(attributeMap, "colour");
    computeTexture->compute();
    attributeMap.getFromDevice();
    
    delete computeTexture;
}

float GameTerrain::getHeight(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*heightTexture, scale, a_pos).x;
}

float GameTerrain::getAttribute(const vec2 &a_pos) const
{
    return sampleTextureBilinear(*attributeTexture, scale, a_pos).x;
}

