/*
Copyright 2015, Matthijs van Dorp.

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
#include <vector>
#include <string>
#include <exception>

#include <config.h>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include <tiny/img/io/image.h>
#include <tiny/mesh/io/staticmesh.h>

#include <tiny/lod/quadtree.h>

#include <tiny/draw/texture2d.h>
#include <tiny/draw/texture2darray.h>
#include <tiny/draw/computetexture.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/tiledhorde.h>
#include <tiny/draw/terrain.h>
#include <tiny/draw/heightmap/scale.h>
#include <tiny/draw/heightmap/resize.h>
#include <tiny/draw/heightmap/normalmap.h>
#include <tiny/draw/heightmap/tangentmap.h>
#include <tiny/draw/heightmap/diamondsquare.h>
#include <tiny/draw/effects/sunsky.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

//All terrain-related data.
const vec2 terrainScale = vec2(3.0f, 3.0f);
const float terrainHeightScale = 2617.0f;
draw::Terrain *terrain = 0;
draw::FloatTexture2D *terrainHeightTexture = 0;
draw::RGBTexture2D *terrainNormalTexture = 0;
draw::RGBTexture2D *terrainTangentTexture = 0;

const vec2 terrainDiffuseScale = vec2(1024.0f, 1024.0f);
draw::RGBATexture2D *terrainAttributeTexture = 0;
draw::RGBTexture2DArray *terrainLocalDiffuseTextures = 0;
draw::RGBTexture2DArray *terrainLocalNormalTextures = 0;

const ivec2 terrainFarScale = ivec2(16, 16);
const vec2 terrainFarOffset = vec2(0.5f, 0.5f);
draw::FloatTexture2D *terrainFarHeightTexture = 0;
draw::RGBTexture2D *terrainFarNormalTexture = 0;
draw::RGBTexture2D *terrainFarTangentTexture = 0;

draw::RGBATexture2D *terrainFarAttributeTexture = 0;

//Forest data.
draw::TiledHorde * tiledForest = 0;
const float tileSize = 32.0f;

const int maxNrHighDetailTrees = 11024;
const int maxNrLowDetailTrees = 132768;
const float treeHighDetailRadius = 96.0f;
const float treeLowDetailRadius = 512.0f;
draw::StaticMeshHorde *treeTrunkMeshes = 0;
draw::StaticMeshHorde *treeLeavesMeshes = 0;
draw::WorldIconHorde *treeSprites = 0;
draw::RGBTexture2D *treeTrunkDiffuseTexture = 0;
draw::RGBTexture2D *treeTrunkNormalTexture = 0;
draw::RGBATexture2D *treeLeavesDiffuseTexture = 0;
draw::RGBATexture2D *treeSpriteTexture = 0;

//Sky box and associated atmospherics data.
draw::StaticMesh *skyBox = 0;
draw::RGBTexture2D *skyBoxTexture = 0;
draw::RGBATexture2D *skyGradientTexture = 0;
float sunAngle = -0.4f;

draw::effects::SunSky *sunSky = 0;

//Camera data.
bool lodFollowsCamera = true;

vec3 cameraPosition = vec3(0.001f, 256.0f, 0.001f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

//A GLSL program that determines the terrain type (forest/grass/mud/stone) from height information on the terrain.
template<typename TextureType1, typename TextureType2>
void computeTerrainTypeFromHeight(const TextureType1 &heightMap, TextureType2 &colourMap, const float &mapScale)
{
	std::vector<std::string> inputTextures;
	std::vector<std::string> outputTextures;
	const std::string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceInverseSize;\n"
"uniform float mapScale;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   float height = texture(source, tex).x;\n"
"   float east = texture(source, tex + vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float west = texture(source, tex - vec2(sourceInverseSize.x, 0.0f)).x;\n"
"   float north = texture(source, tex + vec2(0.0f, sourceInverseSize.y)).x;\n"
"   float south = texture(source, tex - vec2(0.0f, sourceInverseSize.y)).x;\n"
"   \n"
"   vec3 normal = normalize(vec3(west - east, mapScale, south - north));\n"
"   \n"
"	float slope = 1.0f - normal.y;\n"
"	float forestFrac = clamp(max(0.0f, 1.0f - 9.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 450.0f)), 0.0f, 1.0f);\n"
"	float grassFrac = (1.0f - forestFrac)*clamp(max(0.0f, 1.0f - 7.0f*slope)*max(0.0f, 1.0f - 0.1f*(height - 1200.0f)), 0.0f, 1.0f);\n"
"	float mudFrac = (1.0f - grassFrac - forestFrac)*clamp(max(0.0f, 1.0f - 1.0f*slope), 0.0f, 1.0f);\n"
"	float rockFrac = 1.0f - forestFrac - grassFrac - mudFrac;\n"
"	\n"
"   colour = vec4((0.0f*forestFrac + 1.0f*grassFrac + 2.0f*mudFrac + 3.0f*rockFrac)/255.0f, 0.0f, 0.0f, 0.0f);\n"
"}\n";
	
	inputTextures.push_back("source");
	outputTextures.push_back("colour");

	draw::ComputeTexture *computeTexture = new draw::ComputeTexture(inputTextures, outputTextures, fragmentShader);
	
	computeTexture->uniformMap().setFloatUniform(2.0f*mapScale, "mapScale");
	computeTexture->setInput(heightMap, "source");
	computeTexture->setOutput(colourMap, "colour");
	computeTexture->compute();
	colourMap.getFromDevice();
	
	delete computeTexture;
}

//A simple bilinear texture sampler, which converts world coordinates to the corresponding texture coordinates on the zoomed-in terrain.
template<typename TextureType>
vec4 sampleTextureBilinear(const TextureType &texture, const vec2 &scale, const vec2 &a_pos)
{
	//Sample texture at the four points surrounding pos.
	const vec2 pos = vec2(a_pos.x/scale.x + 0.5f*static_cast<float>(texture.getWidth()), a_pos.y/scale.y + 0.5f*static_cast<float>(texture.getHeight()));
	const ivec2 intPos = ivec2(floor(pos.x), floor(pos.y));
	const vec4 h00 = texture(intPos.x + 0, intPos.y + 0);
	const vec4 h01 = texture(intPos.x + 0, intPos.y + 1);
	const vec4 h10 = texture(intPos.x + 1, intPos.y + 0);
	const vec4 h11 = texture(intPos.x + 1, intPos.y + 1);
	const vec2 delta = vec2(pos.x - floor(pos.x), pos.y - floor(pos.y));
	
	//Interpolate between these four points.
	return delta.y*(delta.x*h11 + (1.0f - delta.x)*h01) + (1.0f - delta.y)*(delta.x*h10 + (1.0f - delta.x)*h00);
}

//Function to populate a specific tile with trees.
template<typename TextureType1, typename TextureType2>
int plantTreesTiled(const TextureType1 &heightTexture, const TextureType2 &attributeTexture, const vec2 &scale,
			   const vec3 &origin, const int &maxNrTrees,
			   std::vector<draw::StaticMeshInstance> &highDetailInstances, std::vector<draw::WorldIconInstance> &lowDetailInstances, std::vector<vec3> &positions)
{
	int nrTrees = 0;
	
	highDetailInstances.clear();
	lowDetailInstances.clear();
	positions.clear();
	
	if (maxNrTrees <= 0)
	{
		std::cerr << "Warning: Not placing any trees!" << std::endl;
		return 0;
	}

//	std::cerr << "Placing up to " << maxNrTrees << " trees..." << std::endl;
	
	highDetailInstances.reserve(maxNrTrees);
	lowDetailInstances.reserve(maxNrTrees);
	positions.reserve(maxNrTrees);
	
	for (int i = 0; i < maxNrTrees; ++i)
	{
		//Determine the random spot where we want to place a tree.
		const vec2 treePlanePosition = randomVec2(0.5*tileSize) + vec2(origin.x+0.5*tileSize, origin.z+0.5*tileSize);
		
		//Are we going to place a tree here?
		const float placeProbability = sampleTextureBilinear(attributeTexture, scale, treePlanePosition).x;
		
		if (placeProbability <= 0.5f/255.0f)
		{
			//Determine height.
			const vec3 treePosition = vec3(treePlanePosition.x, sampleTextureBilinear(heightTexture, scale, treePlanePosition).x, treePlanePosition.y);
			
			highDetailInstances.push_back(draw::StaticMeshInstance(vec4(treePosition.x, treePosition.y, treePosition.z, 1.0f),
																   vec4(0.0f, 0.0f, 0.0f, 1.0f)));
			lowDetailInstances.push_back(draw::WorldIconInstance(vec4(treePosition.x, treePosition.y + 4.0f, treePosition.z, 1.0f),
																 vec2(4.0f, 4.0f),
																 vec4(0.0f, 0.0f, 1.0f, 1.0f),
																 vec4(1.0f, 1.0f, 1.0f, 1.0f)));
			positions.push_back(treePosition);
			++nrTrees;
		}
	}

//	std::cerr << "Placed " << nrTrees << " trees." << std::endl;
	
	return nrTrees;
}


//Function to populate a list with trees, that are placed with probability as indicated by a given attribute map.
/*template<typename TextureType1, typename TextureType2>
int plantTrees(const TextureType1 &heightTexture, const TextureType2 &attributeTexture, const vec2 &scale,
			   const float &maxDistance, const int &maxNrTrees,
			   std::vector<draw::StaticMeshInstance> &highDetailInstances, std::vector<draw::WorldIconInstance> &lowDetailInstances, std::vector<vec3> &positions)
{
	int nrTrees = 0;
	
	highDetailInstances.clear();
	lowDetailInstances.clear();
	positions.clear();
	
	if (maxNrTrees <= 0)
	{
		std::cerr << "Warning: Not placing any trees!" << std::endl;
		return 0;
	}

	std::cerr << "Placing up to " << maxNrTrees << " trees..." << std::endl;
	
	highDetailInstances.reserve(maxNrTrees);
	lowDetailInstances.reserve(maxNrTrees);
	positions.reserve(maxNrTrees);
	
	for (int i = 0; i < maxNrTrees; ++i)
	{
		//Determine the random spot where we want to place a tree.
		const vec2 treePlanePosition = randomVec2(maxDistance);
		
		//Are we going to place a tree here?
		const float placeProbability = sampleTextureBilinear(attributeTexture, scale, treePlanePosition).x;
		
		if (placeProbability <= 0.5f/255.0f)
		{
			//Determine height.
			const vec3 treePosition = vec3(treePlanePosition.x, sampleTextureBilinear(heightTexture, scale, treePlanePosition).x, treePlanePosition.y);
			
			highDetailInstances.push_back(draw::StaticMeshInstance(vec4(treePosition.x, treePosition.y, treePosition.z, 1.0f),
																   vec4(0.0f, 0.0f, 0.0f, 1.0f)));
			lowDetailInstances.push_back(draw::WorldIconInstance(vec4(treePosition.x, treePosition.y + 4.0f, treePosition.z, 1.0f),
																 vec2(4.0f, 4.0f),
																 vec4(0.0f, 0.0f, 1.0f, 1.0f),
																 vec4(1.0f, 1.0f, 1.0f, 1.0f)));
			positions.push_back(treePosition);
			++nrTrees;
		}
	}

	std::cerr << "Placed " << nrTrees << " trees." << std::endl;
	
	return nrTrees;
}
*/

void setup()
{
	srand(1234567890);
	
	//Create large example terrain.
	terrain = new draw::Terrain(6, 8);
	
	//Read heightmap for the far-away terrain.
	terrainHeightTexture = new draw::FloatTexture2D(img::io::readImage(DATA_DIRECTORY + "img/512hills.png"), draw::tf::filter);
	terrainFarHeightTexture = new draw::FloatTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight(), draw::tf::filter);
	
	//Scale vertical range of the far-away heightmap.
	draw::computeScaledTexture(*terrainHeightTexture, *terrainFarHeightTexture, vec4(terrainHeightScale/255.0f), vec4(0.0f));
	
	//Zoom into a small area of the far-away heightmap.
	draw::computeResizedTexture(*terrainFarHeightTexture, *terrainHeightTexture,
								vec2(1.0f/static_cast<float>(terrainFarScale.x), 1.0f/static_cast<float>(terrainFarScale.y)),
								terrainFarOffset);
	
	//Apply the diamond-square fractal algorithm to make the zoomed-in heightmap a little less boring.
	draw::computeDiamondSquareRefinement(*terrainHeightTexture, *terrainHeightTexture, terrainFarScale.x);
	
	//Create normal maps for the far-away and zoomed-in heightmaps.
	terrainFarNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
	terrainNormalTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
	terrainFarTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
	terrainTangentTexture = new draw::RGBTexture2D(terrainHeightTexture->getWidth(), terrainHeightTexture->getHeight());
	
	draw::computeNormalMap(*terrainFarHeightTexture, *terrainFarNormalTexture, terrainScale.x*terrainFarScale.x);
	draw::computeNormalMap(*terrainHeightTexture, *terrainNormalTexture, terrainScale.x);
	draw::computeTangentMap(*terrainFarHeightTexture, *terrainFarTangentTexture, terrainScale.x*terrainFarScale.x);
	draw::computeTangentMap(*terrainHeightTexture, *terrainTangentTexture, terrainScale.x);
	
	//Read diffuse textures and make them tileable.
	if (true)
	{
		std::vector<img::Image> diffuseTextures;
		std::vector<img::Image> normalTextures;
		
		diffuseTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/forest.jpg"));
		diffuseTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/grass.jpg"));
		diffuseTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/dirt.jpg"));
		diffuseTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/rocks.jpg"));
		terrainLocalDiffuseTextures = new draw::RGBTexture2DArray(diffuseTextures.begin(), diffuseTextures.end());
		
		normalTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/forest_normal.jpg"));
		normalTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/grass_normal.jpg"));
		normalTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/dirt_normal.jpg"));
		normalTextures.push_back(img::io::readImage(DATA_DIRECTORY + "img/terrain/rocks_normal.jpg"));
		terrainLocalNormalTextures = new draw::RGBTexture2DArray(normalTextures.begin(), normalTextures.end());
	}
	
	//Create an attribute texture that determines the terrain type (forest/grass/mud/stone) based on the altitude and slope.
	//We do this for both the zoomed-in and far-away terrain.
	terrainAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth(), 255, 0, 0, 0));
	terrainFarAttributeTexture = new draw::RGBATexture2D(img::Image::createSolidImage(terrainHeightTexture->getWidth()));
	
	computeTerrainTypeFromHeight(*terrainHeightTexture, *terrainAttributeTexture, terrainScale.x);
	computeTerrainTypeFromHeight(*terrainFarHeightTexture, *terrainFarAttributeTexture, terrainScale.x*terrainFarScale.x);
	terrainAttributeTexture->getFromDevice();
	
	//Paint the terrain with the zoomed-in and far-away textures.
	terrain->setFarHeightTextures(*terrainHeightTexture, *terrainFarHeightTexture,
								  *terrainTangentTexture, *terrainFarTangentTexture,
								  *terrainNormalTexture, *terrainFarNormalTexture,
								  terrainScale, terrainFarScale, terrainFarOffset);
	terrain->setFarDiffuseTextures(*terrainAttributeTexture, *terrainFarAttributeTexture,
								   *terrainLocalDiffuseTextures, *terrainLocalNormalTextures,
								   terrainDiffuseScale);
	
	//Create a forest by using the attribute texture, only on the zoomed-in terrain.
	//Read and paint the tree trunks.
	treeTrunkMeshes = new draw::StaticMeshHorde(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/tree0_trunk.obj"), maxNrHighDetailTrees);
	treeTrunkDiffuseTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_trunk.png"));
	treeTrunkNormalTexture = new draw::RGBTexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_trunk_normal.png"));
	treeTrunkMeshes->setDiffuseTexture(*treeTrunkDiffuseTexture);
	treeTrunkMeshes->setNormalTexture(*treeTrunkNormalTexture);
	
	//Read and paint the tree leaves.
	treeLeavesMeshes = new draw::StaticMeshHorde(mesh::io::readStaticMesh(DATA_DIRECTORY + "mesh/tree0_leaves.obj"), maxNrHighDetailTrees);
	treeLeavesDiffuseTexture = new draw::RGBATexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_leaves.png"));
	treeLeavesMeshes->setDiffuseTexture(*treeLeavesDiffuseTexture);
	
	//Read and paint the sprites for far-away trees.
	treeSprites = new draw::WorldIconHorde(maxNrLowDetailTrees, false);
	treeSpriteTexture = new draw::RGBATexture2D(img::io::readImage(DATA_DIRECTORY + "img/tree0_sprite.png"));
	treeSprites->setIconTexture(*treeSpriteTexture);
	
	//Create a forest and place it into a quadtree for efficient rendering.
//	std::vector<vec3> tmpTreePositions;
	
//	plantTrees(*terrainHeightTexture, *terrainAttributeTexture, terrainScale,
//			   0.5f*terrainHeightTexture->getWidth()*terrainScale.x, 524288,
//			   allTreeHighDetailInstances, allTreeLowDetailInstances, tmpTreePositions);
	
//	visibleTreeInstanceIndices.resize(std::max(maxNrHighDetailTrees, maxNrLowDetailTrees));
//	visibleTreeHighDetailInstances.resize(maxNrHighDetailTrees);
//	visibleTreeLowDetailInstances.resize(maxNrLowDetailTrees);
	
//	quadtree = new lod::Quadtree();
//	quadtree->buildQuadtree(tmpTreePositions.begin(), tmpTreePositions.end());

	// Create a tiled forest.
	tiledForest = new draw::TiledHorde(tileSize, "Forest");
	std::vector<draw::StaticMeshHorde*> mediumTreeMeshes;
	std::vector<draw::WorldIconHorde*> farTreeMeshes;
	mediumTreeMeshes.push_back(treeTrunkMeshes);
	mediumTreeMeshes.push_back(treeLeavesMeshes);
	farTreeMeshes.push_back(treeSprites);
	
	tiledForest->addLOD(mediumTreeMeshes, treeHighDetailRadius);
	tiledForest->addLOD(farTreeMeshes, treeLowDetailRadius);
	
	//Create sky (a simple cube containing the world).
	skyBox = new draw::StaticMesh(mesh::StaticMesh::createCubeMesh(-1.0e6));
	skyBoxTexture = new draw::RGBTexture2D(img::Image::createSolidImage(16), draw::tf::filter);
	skyBox->setDiffuseTexture(*skyBoxTexture);
	
	//Render using a more advanced shading model.
	sunSky = new draw::effects::SunSky();
	skyGradientTexture = new draw::RGBATexture2D(img::io::readImage(DATA_DIRECTORY + "img/sky.png"));
	sunSky->setSkyTexture(*skyGradientTexture);
	
	//Create a renderer and add the terrain, forest, and the atmospheric rendering effect to it.
	worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
	
	worldRenderer->addWorldRenderable(skyBox);
	
	worldRenderer->addWorldRenderable(terrain);
	
	worldRenderer->addWorldRenderable(treeTrunkMeshes);
	worldRenderer->addWorldRenderable(treeLeavesMeshes);
	worldRenderer->addWorldRenderable(treeSprites);
	
	worldRenderer->addScreenRenderable(sunSky, false, false);
}

void cleanup()
{
	delete worldRenderer;
	
	delete sunSky;
	
	delete skyBox;
	delete skyBoxTexture;
	delete skyGradientTexture;
	
	delete quadtree;
	delete treeTrunkMeshes;
	delete treeLeavesMeshes;
	delete treeSprites;
	delete treeTrunkDiffuseTexture;
	delete treeTrunkNormalTexture;
	delete treeLeavesDiffuseTexture;
	delete treeSpriteTexture;
	
	delete terrain;
	
	delete terrainFarHeightTexture;
	delete terrainHeightTexture;
	delete terrainFarTangentTexture;
	delete terrainTangentTexture;
	delete terrainFarNormalTexture;
	delete terrainNormalTexture;
	
	delete terrainFarAttributeTexture;
	delete terrainAttributeTexture;
	delete terrainLocalDiffuseTextures;
	delete terrainLocalNormalTextures;
}

void update(const double &dt)
{
	//Move the camera around.
	application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
	
	//If the camera is below the terrain, increase its height.
	const float terrainHeight = sampleTextureBilinear(*terrainHeightTexture, terrainScale, vec2(cameraPosition.x, cameraPosition.z)).x + 2.0f;
	
	cameraPosition.y = std::max(cameraPosition.y, terrainHeight);
	
	//Test whether we want the terrain to follow the camera.
	if (application->isKeyPressed('1'))
	{
		lodFollowsCamera = true;
	}
	else if (application->isKeyPressed('2'))
	{
		lodFollowsCamera = false;
	}
	
	//Update the sun.
	if (application->isKeyPressed('3'))
	{
		sunAngle -= 1.0f*dt;
	}
	else if (application->isKeyPressed('4'))
	{
		sunAngle += 1.0f*dt;
	}
	
	sunSky->setSun(vec3(sin(sunAngle), cos(sunAngle), 0.5f));
	
	if (lodFollowsCamera)
	{
		//Update the terrain with respect to the camera.
		terrain->setCameraPosition(cameraPosition);

		tiledForest->removeOldTiles(cameraPosition);
		tiledForest->listNewTiles(cameraPosition);

		vec3 tilepos;
		while(tiledForest->getNewStaticMeshTile(tilepos))
		{
			std::vector<draw::StaticMeshInstance> nearTrees;
			std::vector<draw::WorldIconInstance> farTrees;
			std::vector<vec3> tmpTreePositions;
			plantTreesTiled(*terrainHeightTexture, *terrainAttributeTexture, terrainScale,
					   tilepos, 100,
					   nearTrees, farTrees, tmpTreePositions);
			tiledForest->addTile(tilepos,nearTrees);
		}
		while(tiledForest->getNewIconTile(tilepos))
		{
			std::vector<draw::StaticMeshInstance> nearTrees;
			std::vector<draw::WorldIconInstance> farTrees;
			std::vector<vec3> tmpTreePositions;
			plantTreesTiled(*terrainHeightTexture, *terrainAttributeTexture, terrainScale,
					   tilepos, 100,
					   nearTrees, farTrees, tmpTreePositions);
			tiledForest->addTile(tilepos,farTrees);
		}
		tiledForest->recalculateLOD(cameraPosition);
/*		//Update the forest with respect to the camera.
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
		treeSprites->setIcons(visibleTreeLowDetailInstances.begin(), visibleTreeLowDetailInstances.begin() + nrInstances);*/
	}
	
	//Tell the world renderer that the camera has changed.
	worldRenderer->setCamera(cameraPosition, cameraOrientation);
}

void render()
{
	worldRenderer->clearTargets();
	worldRenderer->render();
}

int main(int, char **)
{
	try
	{
		application = new os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
		setup();
	}
	catch (std::exception &e)
	{
		cerr << "Unable to start application!" << endl;
		return -1;
	}
	
	while (application->isRunning())
	{
		update(application->pollEvents());
		render();
		application->paint();
	}
	
	cleanup();
	delete application;
	
	cerr << "Goodbye." << endl;
	
	return 0;
}

