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

#include <tiny/draw/worldrenderer.h>

using namespace tiny::draw;

WorldRenderer::WorldRenderer(const int &screenWidth, const int &screenHeight) :
    aspectRatio(static_cast<float>(screenWidth)/std::max(static_cast<float>(screenHeight), 1.0f)),
    diffuseTexture(screenWidth, screenHeight),
    worldNormalTexture(screenWidth, screenHeight),
    worldPositionTexture(screenWidth, screenHeight),
    depthTexture(screenWidth, screenHeight),
    worldToScreenRenderer(aspectRatio),
    screenToColourRenderer(aspectRatio)
{
    //Link the two rendering stages.
    worldToScreenRenderer.setDiffuseTarget(diffuseTexture);
    worldToScreenRenderer.setNormalsTarget(worldNormalTexture);
    worldToScreenRenderer.setPositionsTarget(worldPositionTexture);
    worldToScreenRenderer.setDepthTextureTarget(depthTexture);
    
    screenToColourRenderer.setDiffuseSource(diffuseTexture);
    screenToColourRenderer.setNormalsSource(worldNormalTexture);
    screenToColourRenderer.setPositionsSource(worldPositionTexture);
}

WorldRenderer::~WorldRenderer()
{

}

void WorldRenderer::setProjectionMatrix(const mat4 &matrix)
{
    worldToScreenRenderer.setProjectionMatrix(matrix);
    screenToColourRenderer.setProjectionMatrix(matrix);
}

void WorldRenderer::setCamera(const vec3 &position, const vec4 &orientation)
{
    worldToScreenRenderer.setCamera(position, orientation);
    screenToColourRenderer.setCamera(position, orientation);
}

void WorldRenderer::addWorldRenderable(Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode)
{
    worldToScreenRenderer.addRenderable(renderable, readFromDepthTexture, writeToDepthTexture, blendMode);
}

void WorldRenderer::addScreenRenderable(Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode)
{
    screenToColourRenderer.addRenderable(renderable, readFromDepthTexture, writeToDepthTexture, blendMode);
}

void WorldRenderer::clearTargets() const
{
    worldToScreenRenderer.clearTargets();
}

void WorldRenderer::render() const
{
    worldToScreenRenderer.render();
    screenToColourRenderer.render();
}

