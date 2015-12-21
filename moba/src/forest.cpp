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

using namespace moba;
using namespace tiny;

GameForest::GameForest(const std::string &path, TiXmlElement *el)
{
    std::cerr << "Reading forest resources..." << std::endl;
    
    assert(el);
    
    img::Image diffuseImage = img::Image::createSolidImage();
    img::Image spriteImage = img::Image::createSolidImage();
    
    mesh::StaticMesh mesh = mesh::StaticMesh::createCubeMesh();

    assert(el->ValueStr() == "forest");
    
    maxNrHighDetailTrees = 1024;
    maxNrLowDetailTrees = 32768;
    nrPlantedTrees = maxNrLowDetailTrees;
    collisionRadius = 0.5f;
    treeHighDetailRadius = 128.0f;
    treeLowDetailRadius = 1024.0f;
    biomeIndex = 0;
    treeSpriteSize = vec2(4.0f, 4.0f);
    
    el->QueryFloatAttribute("collision_radius", &collisionRadius);
    el->QueryFloatAttribute("high_detail_radius", &treeHighDetailRadius);
    el->QueryFloatAttribute("low_detail_radius", &treeLowDetailRadius);
    el->QueryIntAttribute("nr_planted_trees", &nrPlantedTrees);
    el->QueryIntAttribute("nr_high_detail", &maxNrHighDetailTrees);
    el->QueryIntAttribute("nr_low_detail", &maxNrLowDetailTrees);
    el->QueryIntAttribute("biome_index", &biomeIndex);
    el->QueryFloatAttribute("sprite_w", &treeSpriteSize.x);
    el->QueryFloatAttribute("sprite_h", &treeSpriteSize.y);
    
    if (el->Attribute("mesh")) mesh = mesh::io::readStaticMesh(path + std::string(el->Attribute("mesh")));
    if (el->Attribute("diffuse")) diffuseImage = img::io::readImage(path + std::string(el->Attribute("diffuse")));
    if (el->Attribute("sprite")) spriteImage = img::io::readImage(path + std::string(el->Attribute("sprite")));
    
    //High-detail meshes.
    treeMeshes = new draw::StaticMeshHorde(mesh, maxNrHighDetailTrees);
    treeDiffuseTexture = new draw::RGBATexture2D(diffuseImage);
    treeMeshes->setDiffuseTexture(*treeDiffuseTexture);
    
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
    delete treeMeshes;
    delete treeSprites;
    delete treeDiffuseTexture;
    delete treeSpriteTexture;
}

std::list<vec4> GameForest::plantTrees(const GameTerrain *terrain)
{
    //Plan trees on terrain.
    const int maxNrTrees = nrPlantedTrees;
    
    terrain->createAttributeMapSamples(maxNrTrees, biomeIndex, allTreeHighDetailInstances, treeSpriteSize, allTreeLowDetailInstances, treePositions);
    quadtree->buildQuadtree(treePositions.begin(), treePositions.end());
    
    //Add collision cylinders.
    std::list<vec4> cylinders;
    
    for (std::vector<vec3>::const_iterator i = treePositions.begin(); i != treePositions.end(); ++i)
    {
        cylinders.push_back(vec4(i->x, i->y, i->z, collisionRadius));
    }
    
    return cylinders;
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
    treeMeshes->setMeshes(visibleTreeHighDetailInstances.begin(), visibleTreeHighDetailInstances.begin() + nrInstances);
    
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

