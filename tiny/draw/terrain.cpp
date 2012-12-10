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
#include <climits>

#include <tiny/draw/terrain.h>

using namespace tiny;
using namespace tiny::draw;
using namespace tiny::draw::detail;

TerrainBlockVertexBufferInterpreter::TerrainBlockVertexBufferInterpreter(const size_t &n) :
    VertexBufferInterpreter<vec2>(n*n)
{
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            hostData[j + n*i] = vec2(j, i);
        }
    }
    
    sendToDevice();
    
    addVec2Attribute(0, "v_vertex");
}

TerrainBlockVertexBufferInterpreter::~TerrainBlockVertexBufferInterpreter()
{
    
}

TerrainBlockInstanceBufferInterpreter::TerrainBlockInstanceBufferInterpreter(const size_t &maxNrInstances) :
    VertexBufferInterpreter<TerrainBlockInstance>(maxNrInstances)
{
    addVec4Attribute(0*sizeof(float), "v_scaleAndTranslate");
}

TerrainBlockInstanceBufferInterpreter::~TerrainBlockInstanceBufferInterpreter()
{

}

TerrainBlockIndexBuffer::TerrainBlockIndexBuffer(const size_t &n) :
    IndexBuffer<unsigned int>(2*(n + 1)*(n - 1))
{
    size_t count = 0;

    for (size_t i = 0; i < n - 1; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            hostData[count++] = j + n*i;
            hostData[count++] = j + n*(i + 1);
        }

        hostData[count++] = UINT_MAX;
    }
    
    sendToDevice();
}

TerrainBlock::TerrainBlock(const size_t &n, const size_t &maxNrInstances) :
    vertices(n),
    instances(maxNrInstances),
    indices(n)
{

}

TerrainBlock::~TerrainBlock()
{

}

TerrainBlockInstance &TerrainBlock::operator [] (const size_t &index)
{
    return instances[index];
}

const TerrainBlockInstance &TerrainBlock::operator [] (const size_t &index) const
{
    return instances[index];
}

void TerrainBlock::bind(const ShaderProgram &program) const
{
    vertices.bind(program);
    instances.bind(program, 1);
    indices.bind(); 
}

void TerrainBlock::unbind(const ShaderProgram &program) const
{
    indices.unbind();
    instances.unbind(program);
    vertices.unbind(program);
}

