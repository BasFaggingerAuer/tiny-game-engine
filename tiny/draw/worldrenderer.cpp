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

WorldRenderer::WorldRenderer() :
    cameraToScreen(mat4::frustumMatrix(vec3(-0.07, -0.07, 1.0e-1), vec3(0.07, 0.07, 1.0e3))),
    worldToCamera(mat4::identityMatrix()),
    worldToScreen(mat4::identityMatrix()),
    cameraPosition(0.0f, 0.0f, 0.0f)
{
    addRenderTarget("diffuse");
    addRenderTarget("worldNormal");
    addRenderTarget("worldPosition");
    updateUniforms();
}

WorldRenderer::~WorldRenderer()
{

}

void WorldRenderer::setProjectionMatrix(const mat4 &matrix)
{
    cameraToScreen = matrix;
    updateUniforms();
}

void WorldRenderer::setCamera(const vec3 &position, const vec4 &orientation)
{
    worldToCamera = mat4(orientation, position).inverted();
    cameraPosition = position;
    updateUniforms();
}

void WorldRenderer::updateUniforms()
{
    worldToScreen = cameraToScreen*worldToCamera;
    //uniformMap.setVec3Uniform(cameraPosition, "cameraPosition");
    uniformMap.setMat4Uniform(worldToScreen, "worldToScreen");
}

