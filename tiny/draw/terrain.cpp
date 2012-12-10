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

TerrainBlockVertexBuffer::TerrainBlockVertexBuffer(const size_t &n) :
    VertexBuffer<vec2>(n*n)
{
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            hostData[j + n*i] = vec2(j, i);
        }
    }
    
    sendToDevice();
}

TerrainBlockVertexBuffer::~TerrainBlockVertexBuffer()
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

TerrainBlockVertexBufferInterpreter::TerrainBlockVertexBufferInterpreter(const size_t &n, const size_t &maxNrInstances) :
    VertexBufferInterpreter(),
    vertices(n),
    instances(maxNrInstances)
{
    addVec4Attribute(instances, 0*sizeof(float), "scaleAndTranslate");
}

TerrainBlockVertexBufferInterpreter::~TerrainBlockVertexBufferInterpreter()
{

}

TerrainBlock::TerrainBlock(const size_t &n, const size_t &maxNrInstances) :
    vertices(n, maxNrInstances),
    indices(n)
{

}

TerrainBlock::~TerrainBlock()
{

}

TerrainBlockInstance &TerrainBlock::operator [] (const size_t &index)
{
    return vertices.instances[index];
}

const TerrainBlockInstance &TerrainBlock::operator [] (const size_t &index) const
{
    return vertices.instances[index];
}

void TerrainBlock::bind(const ShaderProgram &program) const
{
    
}

void TerrainBlock::unbind(const ShaderProgram &program) const
{

}


