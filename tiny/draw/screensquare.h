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
#pragma once

#include <exception>
#include <vector>
#include <string>
#include <map>

#include <cassert>

#include <tiny/math/vec.h>
#include <tiny/draw/texture.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/renderer.h>

namespace tiny
{

namespace draw
{

class ScreenFillingSquareVertexBufferInterpreter : public VertexBufferInterpreter<vec2>
{
    public:
        ScreenFillingSquareVertexBufferInterpreter();
        ~ScreenFillingSquareVertexBufferInterpreter();

        void setSquareDimensions(float left, float top, float right, float bottom);
};

class ScreenFillingSquare : public Renderable
{
    public:
        ScreenFillingSquare();
        ~ScreenFillingSquare();
        std::string getTypeName() const;
        
        std::string getVertexShaderCode() const;

        void setSquareDimensions(float left, float top, float right, float bottom);
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        ScreenFillingSquareVertexBufferInterpreter square;
};

}

}

