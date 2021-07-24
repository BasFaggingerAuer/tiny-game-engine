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

#include <cassert>

#include <tiny/mesh/staticmesh.h>
#include <tiny/draw/indexbuffer.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/staticmesh.h>
#include <tiny/draw/staticmeshhorde.h>

namespace tiny
{

namespace draw
{

//StaticMeshInstance.colorMultiplier.w is used to index the layer for which the texture array is sampled. XYZ are still used for color scaling.
class StaticMeshHordeTextureArray : public Renderable
{
    public:
        StaticMeshHordeTextureArray(const tiny::mesh::StaticMesh &, const size_t &);
        ~StaticMeshHordeTextureArray();
        std::string getTypeName() const;
        
        template <typename TextureType>
        void setDiffuseTextureArray(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "diffuseTextureArray");
        }
        
        template <typename TextureType>
        void setNormalTextureArray(const TextureType &texture)
        {
            uniformMap.setTexture(texture, "normalTextureArray");
        }
        
        template <typename Iterator>
        void setMeshes(Iterator first, Iterator last)
        {
            setInstances(first, last);
        }

        template <typename Iterator>
        void setInstances(Iterator first, Iterator last)
        {
            nrMeshes = 0;
            
            for (Iterator i = first; i != last && nrMeshes < maxNrMeshes; ++i)
            {
                meshes[nrMeshes++] = *i;
            }
            
            meshes.sendToDevice();
        }
        
        std::string getVertexShaderCode() const;
        std::string getFragmentShaderCode() const;
        
    protected:
        void render(const ShaderProgram &) const;
        
    private:
        const size_t nrVertices;
        const size_t nrIndices;
        const size_t maxNrMeshes;
        size_t nrMeshes;
        
        StaticMeshIndexBuffer indices;
        StaticMeshVertexBufferInterpreter vertices;
        StaticMeshInstanceVertexBufferInterpreter meshes;
};

}

}

