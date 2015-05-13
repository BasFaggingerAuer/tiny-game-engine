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

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/rendererwithcamera.h>
#include <tiny/draw/texture2d.h>
#include <tiny/draw/detail/worldrenderer.h>

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
        
        void addWorldRenderable(Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace);
        void addScreenRenderable(Renderable *, const bool & = true, const bool & = true, const BlendMode & = BlendReplace);

		void freeRenderable(Renderable *);
        
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

}

}

