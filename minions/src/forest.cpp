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
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/draw/terrain.h>

#include "forest.h"

using namespace minions;
using namespace tiny;

GameForest::GameForest(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading forest resources..." << std::endl;
    
    assert(el);
    
    img::Image trunkDiffuseImage = img::Image::createSolidImage();
    img::Image trunkNormalImage = img::Image::createUpNormalImage();
    img::Image leavesDiffuseImage = img::Image::createSolidImage();
    img::Image spriteImage = img::Image::createSolidImage();
    
    mesh::StaticMesh trunkMesh = mesh::StaticMesh::createCylinderMesh();
    mesh::StaticMesh leavesMesh = mesh::StaticMesh::createCubeMesh();

    assert(el->ValueStr() == "forest");
    
    maxNrHighDetailTrees = 1024;
    maxNrLowDetailTrees = 32768;
    treeHighDetailRadius = 128.0;
    treeLowDetailRadius = 1024.0;
    biomeIndex = 0;
    treeSpriteSize = vec2(4.0f, 4.0f);
    
    el->QueryFloatAttribute("high_detail_radius", &treeHighDetailRadius);
    el->QueryFloatAttribute("low_detail_radius", &treeLowDetailRadius);
    el->QueryIntAttribute("nr_high_detail_trees", &maxNrHighDetailTrees);
    el->QueryIntAttribute("nr_low_detail_trees", &maxNrLowDetailTrees);
    el->QueryIntAttribute("biome_index", &biomeIndex);
    el->QueryFloatAttribute("sprite_w", &treeSpriteSize.x);
    el->QueryFloatAttribute("sprite_h", &treeSpriteSize.y);
    
    if (el->Attribute("trunk_mesh")) trunkMesh = mesh::io::readStaticMesh(path + std::string(el->Attribute("trunk_mesh")));
    if (el->Attribute("trunk_diffuse")) trunkDiffuseImage = img::io::readImage(path + std::string(el->Attribute("trunk_diffuse")));
    if (el->Attribute("trunk_normal")) trunkNormalImage = img::io::readImage(path + std::string(el->Attribute("trunk_normal")));
    if (el->Attribute("leaves_mesh")) leavesMesh = mesh::io::readStaticMesh(path + std::string(el->Attribute("leaves_mesh")));
    if (el->Attribute("leaves_diffuse")) leavesDiffuseImage = img::io::readImage(path + std::string(el->Attribute("leaves_diffuse")));
    if (el->Attribute("sprite")) spriteImage = img::io::readImage(path + std::string(el->Attribute("sprite")));
    
    //High-detail trunks.
    treeTrunkMeshes = new draw::StaticMeshHorde(trunkMesh, maxNrHighDetailTrees);
    treeTrunkDiffuseTexture = new draw::RGBTexture2D(trunkDiffuseImage);
    treeTrunkNormalTexture = new draw::RGBTexture2D(trunkNormalImage);
    treeTrunkMeshes->setDiffuseTexture(*treeTrunkDiffuseTexture);
    treeTrunkMeshes->setNormalTexture(*treeTrunkNormalTexture);
    
    //High-detail leaves.
    treeLeavesMeshes = new draw::StaticMeshHorde(leavesMesh, maxNrHighDetailTrees);
    treeLeavesDiffuseTexture = new draw::RGBATexture2D(leavesDiffuseImage);
    treeLeavesMeshes->setDiffuseTexture(*treeLeavesDiffuseTexture);
    
    //Read and paint the sprites for far-away trees.
    treeSprites = new draw::WorldIconHorde(maxNrLowDetailTrees, false);
    treeSpriteTexture = new draw::RGBATexture2D(spriteImage);
    treeSprites->setIconTexture(*treeSpriteTexture);
    
    //Create a forest and place it into a quadtree for efficient rendering.
    visibleTreeInstanceIndices.resize(std::max(maxNrHighDetailTrees, maxNrLowDetailTrees));
    visibleTreeHighDetailInstances.resize(maxNrHighDetailTrees);
    visibleTreeLowDetailInstances.resize(maxNrLowDetailTrees);
    quadtree = new lod::Quadtree();
}

GameForest::~GameForest()
{
    delete quadtree;
    delete treeTrunkMeshes;
    delete treeLeavesMeshes;
    delete treeSprites;
    delete treeTrunkDiffuseTexture;
    delete treeTrunkNormalTexture;
    delete treeLeavesDiffuseTexture;
    delete treeSpriteTexture;
}

int GameForest::plantTrees(const GameTerrain *terrain, const int &maxNrTrees)
{
    //Plan trees on terrain.
    const int nrTrees = terrain->createAttributeMapSamples(maxNrTrees, biomeIndex, allTreeHighDetailInstances, treeSpriteSize, allTreeLowDetailInstances, treePositions);
    
    quadtree->buildQuadtree(treePositions.begin(), treePositions.end());
    
    return nrTrees;
}

void GameForest::setCameraPosition(const vec3 &cameraPosition)
{
    if (treePositions.empty())
    {
        return;
    }
    
    //Update the forest with respect to the camera.
    int nrInstances = quadtree->retrieveIndicesBetweenRadii(cameraPosition,
                                                            0.0f, treeHighDetailRadius,
                                                            visibleTreeInstanceIndices.begin(), maxNrHighDetailTrees)
                      - visibleTreeInstanceIndices.begin();
    
    //Copy high detail instances.
    for (int i = 0; i < nrInstances; ++i)
    {
        visibleTreeHighDetailInstances[i] = allTreeHighDetailInstances[visibleTreeInstanceIndices[i]];
    }
    
    //Send them to the GPU.
    treeTrunkMeshes->setMeshes(visibleTreeHighDetailInstances.begin(), visibleTreeHighDetailInstances.begin() + nrInstances);
    treeLeavesMeshes->setMeshes(visibleTreeHighDetailInstances.begin(), visibleTreeHighDetailInstances.begin() + nrInstances);
    
    nrInstances = quadtree->retrieveIndicesBetweenRadii(cameraPosition,
                                                        treeHighDetailRadius, treeLowDetailRadius,
                                                        visibleTreeInstanceIndices.begin(), maxNrLowDetailTrees)
                  - visibleTreeInstanceIndices.begin();
    
    //Copy low detail instances.
    for (int i = 0; i < nrInstances; ++i)
    {
        visibleTreeLowDetailInstances[i] = allTreeLowDetailInstances[visibleTreeInstanceIndices[i]];
    }
    
    //Send them to the GPU.
    treeSprites->setIcons(visibleTreeLowDetailInstances.begin(), visibleTreeLowDetailInstances.begin() + nrInstances);
}

