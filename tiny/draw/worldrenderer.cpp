/*
Copyright 2012-2015, Bas Fagginger Auer and Matthijs van Dorp.

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

using namespace tiny;
using namespace tiny::draw;

WorldRenderer::WorldRenderer(const int &screenWidth, const int &screenHeight) :
    aspectRatio(static_cast<float>(screenWidth)/std::max(static_cast<float>(screenHeight), 1.0f)),
    diffuseTexture(screenWidth, screenHeight, tf::none),
    worldNormalTexture(screenWidth, screenHeight, tf::none),
    worldPositionTexture(screenWidth, screenHeight, tf::none),
    depthTexture(screenWidth, screenHeight),
    worldToScreenRenderer(aspectRatio, screenWidth, screenHeight),
    screenToColourRenderer(aspectRatio, screenWidth, screenHeight)
{
    //Link the two rendering stages.
    worldToScreenRenderer.setDiffuseTarget(diffuseTexture);
    worldToScreenRenderer.setNormalsTarget(worldNormalTexture);
    worldToScreenRenderer.setPositionsTarget(worldPositionTexture);
    worldToScreenRenderer.setDepthTextureTarget(depthTexture);
    
    screenToColourRenderer.setDiffuseSource(diffuseTexture);
    screenToColourRenderer.setNormalsSource(worldNormalTexture);
    screenToColourRenderer.setPositionsSource(worldPositionTexture);
    setScreenSize(screenWidth, screenHeight);
}

void WorldRenderer::setScreenSize(const int &screenWidth, const int &screenHeight)
{
    screenToColourRenderer.setScreenSize(screenWidth, screenHeight);
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

vec3 WorldRenderer::getWorldDirection(const vec2 &screenCoordinates) const
{
    return worldToScreenRenderer.getWorldDirection(screenCoordinates);
}

void WorldRenderer::addWorldRenderable(const unsigned int &renderableIndex, Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode, const CullMode &cullMode)
{
    worldToScreenRenderer.addRenderable(renderableIndex, renderable, readFromDepthTexture, writeToDepthTexture, blendMode, cullMode);
}

void WorldRenderer::addScreenRenderable(const unsigned int &renderableIndex, Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode, const CullMode &cullMode)
{
    screenToColourRenderer.addRenderable(renderableIndex, renderable, readFromDepthTexture, writeToDepthTexture, blendMode, cullMode);
}

void WorldRenderer::freeWorldRenderable(const unsigned int &renderableIndex)
{
    worldToScreenRenderer.freeRenderable(renderableIndex);
}

void WorldRenderer::freeScreenRenderable(const unsigned int &renderableIndex)
{
    screenToColourRenderer.freeRenderable(renderableIndex);
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

#ifdef ENABLE_OPENVR

WorldRendererVR::WorldRendererVR(const int &screenWidth, const int &screenHeight, vr::IVRSystem *_vrHMD) :
    aspectRatio(static_cast<float>(screenWidth)/std::max(static_cast<float>(screenHeight), 1.0f)),
    vrHMD(_vrHMD),
    eyeEnums({{vr::Eye_Left, vr::Eye_Right}}),
    diffuseTexture(screenWidth, screenHeight, tf::none),
    worldNormalTexture(screenWidth, screenHeight, tf::none),
    worldPositionTexture(screenWidth, screenHeight, tf::none),
    depthTexture(screenWidth, screenHeight),
    eyeTextures({{RGBATexture2D(screenWidth, screenHeight, tf::filter), RGBATexture2D(screenWidth, screenHeight, tf::filter)}}),
    worldToScreenRenderer(aspectRatio),
    screenToEyeRenderers({{detail::WorldRendererStageTwo(aspectRatio), detail::WorldRendererStageTwo(aspectRatio)}})
{
    //Link the two rendering stages.
    worldToScreenRenderer.setDiffuseTarget(diffuseTexture);
    worldToScreenRenderer.setNormalsTarget(worldNormalTexture);
    worldToScreenRenderer.setPositionsTarget(worldPositionTexture);
    worldToScreenRenderer.setDepthTextureTarget(depthTexture);
    
    for (int i = 0; i < 2; ++i) {
        screenToEyeRenderers[i].setDiffuseSource(diffuseTexture);
        screenToEyeRenderers[i].setNormalsSource(worldNormalTexture);
        screenToEyeRenderers[i].setPositionsSource(worldPositionTexture);
        screenToEyeRenderers[i].setTextureTarget(eyeTextures[i], "colour");
    }

    //Retrieve eye transformation matrices.
    for (int i = 0; i < 2; ++i) {
        eyeProjectionMatrices[i] =  mat4(vrHMD->GetProjectionMatrix(eyeEnums[i], RendererWithCamera::nearClippingPlane, RendererWithCamera::farClippingPlane));
        headToEyeMatrices[i] =  mat4(vrHMD->GetEyeToHeadTransform(eyeEnums[i])).inverted();
    }

    update();
}

WorldRendererVR::~WorldRendererVR()
{

}

void WorldRendererVR::setCamera(const vec3 &position, const vec4 &orientation)
{
    worldToScreenRenderer.setCamera(position, orientation);

    for (int i = 0; i < 2; ++i) {
        screenToEyeRenderers[i].setCamera(position, orientation);
    }
}

void WorldRendererVR::addWorldRenderable(const unsigned int &renderableIndex, Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode, const CullMode &cullMode)
{
    worldToScreenRenderer.addRenderable(renderableIndex, renderable, readFromDepthTexture, writeToDepthTexture, blendMode, cullMode);
}

void WorldRendererVR::addScreenRenderable(const unsigned int &renderableIndex, Renderable *renderable, const bool &readFromDepthTexture, const bool &writeToDepthTexture, const BlendMode &blendMode, const CullMode &cullMode)
{
    for (int i = 0; i < 2; ++i) {
        screenToEyeRenderers[i].addRenderable(renderableIndex, renderable, readFromDepthTexture, writeToDepthTexture, blendMode, cullMode);
    }
}

void WorldRendererVR::freeWorldRenderable(const unsigned int &renderableIndex)
{
    worldToScreenRenderer.freeRenderable(renderableIndex);
}

void WorldRendererVR::freeScreenRenderable(const unsigned int &renderableIndex)
{
    for (int i = 0; i < 2; ++i) {
        screenToEyeRenderers[i].freeRenderable(renderableIndex);
    }
}

void WorldRendererVR::clearTargets() const
{
    //Done during rendering.
}

void WorldRendererVR::update()
{
    vr::VREvent_t event;

    while (vrHMD->PollNextEvent(&event, sizeof(event)))
    {
        //TODO: Interpret events.
        switch (event.eventType)
        {
        case vr::VREvent_TrackedDeviceActivated:
            break;
        case vr::VREvent_TrackedDeviceDeactivated:
            break;
        }
    }

    vr::VRCompositor()->WaitGetPoses(vrDevicePoses, vr::k_unMaxTrackedDeviceCount, 0, 0);

    if (vrDevicePoses[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
    {
        cameraToHeadMatrix = mat4(vrDevicePoses[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking).inverted();
    }
}

void WorldRendererVR::render()
{
    for (int i = 0; i < 2; ++i)
    {
        const mat4 matrix = eyeProjectionMatrices[i]*headToEyeMatrices[i]*cameraToHeadMatrix;

        worldToScreenRenderer.setProjectionMatrix(matrix);
        screenToEyeRenderers[i].setProjectionMatrix(matrix);
    
        worldToScreenRenderer.clearTargets();
        worldToScreenRenderer.render();
        screenToEyeRenderers[i].render();
    }

    for (int i = 0; i < 2; ++i)
    {
        vr::Texture_t eyeTexture = {(void*)(uintptr_t)eyeTextures[i].getIndex(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma};
		vr::EVRCompositorError error = vr::VRCompositor()->Submit(eyeEnums[i], &eyeTexture);

        if (error != vr::VRCompositorError_None)
        {
            std::cerr << "VR compositor submission error " << error << "!" << std::endl;
        }
    }

    GL_CHECK(glFlush());
}

#endif
