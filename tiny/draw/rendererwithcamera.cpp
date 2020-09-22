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

#include <tiny/draw/rendererwithcamera.h>

using namespace tiny;
using namespace tiny::draw;

const float RendererWithCamera::nearClippingPlane = 1.0e-1f;
const float RendererWithCamera::farClippingPlane = 1.0e8f;

RendererWithCamera::RendererWithCamera(const float &aspectRatio) :
    Renderer(),
    cameraToScreen(mat4::frustumMatrix(vec3(-0.07f*aspectRatio, -0.07f, nearClippingPlane), vec3(0.07f*aspectRatio, 0.07f, farClippingPlane))),
    cameraToWorld(mat4::identityMatrix()),
    worldToCamera(mat4::identityMatrix()),
    worldToScreen(mat4::identityMatrix()),
    screenToWorld(mat4::identityMatrix()),
    cameraPosition(0.0f, 0.0f, 0.0f)
{
    updateCameraUniforms();
}

RendererWithCamera::~RendererWithCamera()
{

}

void RendererWithCamera::setProjectionMatrix(const mat4 &matrix)
{
    cameraToScreen = matrix;
    updateCameraUniforms();
}

void RendererWithCamera::setCamera(const vec3 &position, const vec4 &orientation)
{
    cameraToWorld = mat4(orientation, position);
    worldToCamera = cameraToWorld.inverted();
    cameraPosition = position;
    updateCameraUniforms();
}

vec3 RendererWithCamera::getWorldDirection(const vec2 &screenCoordinates) const
{
    //Calculate ray direction in 3D corresponding to screen coordinates normalized to [0, 1] x [0, 1] with the latest updated camera values.
    vec4 tmp = screenToWorld*vec4(2.0f*screenCoordinates - 1.0f, -1.0f, 1.0f);
    
    return normalize(((tmp/tmp.w) - vec4(cameraPosition, 0.0f)).xyz());
}

void RendererWithCamera::updateCameraUniforms()
{
    worldToScreen = cameraToScreen*worldToCamera;
    screenToWorld = worldToScreen.invertedFull();
    
    uniformMap.setVec3Uniform(cameraPosition, "cameraPosition");
    uniformMap.setMat4Uniform(cameraToWorld, "cameraToWorld");
    uniformMap.setMat4Uniform(worldToScreen, "worldToScreen");
    uniformMap.setMat4Uniform(screenToWorld, "screenToWorld");
}

