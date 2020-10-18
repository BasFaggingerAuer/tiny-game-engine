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
#pragma once

#include <iostream>
#include <exception>
#include <list>
#include <array>

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/rendererwithcamera.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/detail/worldrenderer.h>

#ifdef ENABLE_OPENVR
#include <openvr.h>
#endif

namespace tiny
{

namespace draw
{

class WorldRenderer
{
    public:
        WorldRenderer(const int &, const int &);
        ~WorldRenderer();
        
        void setProjectionMatrix(const mat4 &);
        void setCamera(const vec3 &, const vec4 &);
        void setScreenSize(const int &, const int &);
        vec3 getWorldDirection(const vec2 &) const;
        
        void addWorldRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace, const CullMode & = CullBack);
        void addScreenRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace, const CullMode & = CullBack);
        void freeWorldRenderable(const unsigned int &);
        void freeScreenRenderable(const unsigned int &);
        
        void clearTargets() const;
        void render() const;
        
    private:
        const float aspectRatio;
        
        RGBATexture2D diffuseTexture;
        Vec4Texture2D worldNormalTexture;
        Vec4Texture2D worldPositionTexture;
        DepthTexture2D depthTexture;
        
        detail::WorldRendererStageOne worldToScreenRenderer;
        detail::WorldRendererStageTwo screenToColourRenderer;
};

#ifdef ENABLE_OPENVR

class WorldRendererVR
{
    public:
        WorldRendererVR(const int &, const int &, vr::IVRSystem *);
        ~WorldRendererVR();
        
        void setCamera(const vec3 &, const vec4 &);
        
        void addWorldRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace, const CullMode & = CullBack);
        void addScreenRenderable(const unsigned int &, Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace, const CullMode & = CullBack);
        void freeWorldRenderable(const unsigned int &);
        void freeScreenRenderable(const unsigned int &);
        
        void clearTargets() const;
        void update();
        void render();
        
    private:
        float aspectRatio;
        
        vr::IVRSystem *vrHMD;
        vr::TrackedDevicePose_t vrDevicePoses[vr::k_unMaxTrackedDeviceCount];
        const std::array<vr::EVREye, 2> eyeEnums;
        std::array<mat4, 2> eyeProjectionMatrices;
        std::array<mat4, 2> headToEyeMatrices;
        mat4 cameraToHeadMatrix;
        
        RGBATexture2D diffuseTexture;
        Vec4Texture2D worldNormalTexture;
        Vec4Texture2D worldPositionTexture;
        DepthTexture2D depthTexture;
        std::array<RGBATexture2D, 2> eyeTextures;
        
        detail::WorldRendererStageOne worldToScreenRenderer;
        std::array<detail::WorldRendererStageTwo, 2> screenToEyeRenderers;
};

#endif

}

}

