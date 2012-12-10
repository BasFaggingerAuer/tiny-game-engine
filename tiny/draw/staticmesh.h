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

#include <iostream>
#include <exception>
#include <string>
#include <map>

#include <cassert>

#include <tiny/mesh/staticmesh.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/renderable.h>

namespace tiny
{

namespace draw
{

class StaticMeshVertexBufferInterpreter : public VertexBufferInterpreter<tiny::mesh::StaticMeshVertex>
{
    public:
        StaticMeshVertexBufferInterpreter(const tiny::mesh::StaticMesh &);
        ~StaticMeshVertexBufferInterpreter();
};

class StaticMesh : public Renderable
{
    public:
        StaticMesh(const tiny::mesh::StaticMesh &);
        ~StaticMesh();
        
        template <typename TextureType>
        void setDiffuseTexture(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "diffuseTexture");
        }
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const size_t nrVertices;
        const size_t nrIndices;
        
        IndexBuffer<unsigned int> indices;
        StaticMeshVertexBufferInterpreter vertices;
};

}

}

