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
#include <algorithm>

#include <tiny/draw/detail/worldrenderer.h>

using namespace tiny::draw::detail;

WorldRendererStageOne::WorldRendererStageOne(const float &aspectRatio) :
    RendererWithCamera(aspectRatio, false)
{
    addRenderTarget("diffuse");
    addRenderTarget("worldNormal");
    addRenderTarget("worldPosition");
}

WorldRendererStageOne::~WorldRendererStageOne()
{

}

WorldRendererStageTwo::WorldRendererStageTwo(const float &aspectRatio) :
    RendererWithCamera(aspectRatio, true)
{
    addRenderTarget("colour");
    uniformMap.addTexture("diffuseTexture");
    uniformMap.addTexture("worldNormalTexture");
    uniformMap.addTexture("worldPositionTexture");
}

WorldRendererStageTwo::~WorldRendererStageTwo()
{

}

void WorldRendererStageTwo::setScreenSize(const int &screenWidth, const int &screenHeight)
{
    uniformMap.setVec2Uniform(vec2(1.0f/static_cast<float>(screenWidth),
                                   1.0f/static_cast<float>(screenHeight)),
                              "inverseScreenSize");

}