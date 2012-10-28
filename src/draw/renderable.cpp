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

#include <draw/renderable.h>

using namespace tiny::draw;

Renderable::Renderable()
{

}

Renderable::Renderable(const Renderable &a_renderable)
{
    //TODO.
}

Renderable::~Renderable()
{

}

void Renderable::addFloatVariable(const float &x, const std::string &name) {floatUniforms.push_back(FloatUniform(name, 1, x));}
void Renderable::addVec2Variable(const float &x, const float &y, const std::string &name) {floatUniforms.push_back(FloatUniform(name, 2, x, y));}
void Renderable::addVec3Variable(const float &x, const float &y, const float &z, const std::string &name) {floatUniforms.push_back(FloatUniform(name, 3, x, y, z));}
void Renderable::addVec4Variable(const float &x, const float &y, const float &z, const float &w, const std::string &name) {floatUniforms.push_back(FloatUniform(name, 4, x, y, z, w));}

