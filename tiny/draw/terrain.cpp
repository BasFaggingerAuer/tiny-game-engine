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

TerrainBlockVertexBufferInterpreter::TerrainBlockVertexBufferInterpreter(const size_t &width, const size_t &height) :
    VertexBufferInterpreter<vec2>(width*height)
{
    for (size_t i = 0; i < height; ++i)
    {
        for (size_t j = 0; j < width; ++j)
        {
            hostData[j + width*i] = vec2(j, i);
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

TerrainBlockIndexBuffer::TerrainBlockIndexBuffer(const size_t &width, const size_t &height) :
    IndexBuffer<unsigned int>(2*(width + 1)*(height - 1))
{
    size_t count = 0;

    for (size_t i = 0; i < height - 1; ++i)
    {
        for (size_t j = 0; j < width; ++j)
        {
            hostData[count++] = j + width*i;
            hostData[count++] = j + width*(i + 1);
        }

        hostData[count++] = UINT_MAX;
    }
    
    sendToDevice();
}

TerrainBlockIndexBuffer::~TerrainBlockIndexBuffer()
{

}

TerrainBlock::TerrainBlock(const size_t &width, const size_t &height, const size_t &maxNrInstances) :
    vertices(width, height),
    instances(maxNrInstances),
    indices(width, height)
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
}

void TerrainBlock::unbind(const ShaderProgram &program) const
{
    instances.unbind(program);
    vertices.unbind(program);
}

Terrain::Terrain(const int &a_shiftBlockSize, const int &a_maxLevel) :
    Renderable(),
    minLevel(0),
    maxLevel(a_maxLevel > 2 ? a_maxLevel : 2),
    blockSize(1 << a_shiftBlockSize),
    superBlockSize(4*blockSize + 2),
    scale(vec3(1.0f, 1.0f, 1.0f)),
    bitShifts(ivec2(0, 0)),
    blockTranslations(maxLevel, ivec2(0, 0)),
    smallBlock(blockSize, blockSize, 12*maxLevel),
    largeBlock(2*blockSize + 3, 2*blockSize + 3, maxLevel),
    crossBlockX(blockSize, 5, 2*maxLevel),
    crossBlockY(blockSize, 5, 2*maxLevel),
    ellBlockX(2*blockSize + 3, 2, maxLevel),
    ellBlockY(2, 2*blockSize + 3, maxLevel)
{
    //Setup initial blockTranslations.
    blockTranslations[maxLevel - 1] = ivec2(-(superBlockSize << (maxLevel - 2)));
    
    for (int i = maxLevel - 2; i >= 0; --i)
    {
        blockTranslations[i] = blockTranslations[i + 1] + ivec2((2*blockSize - 1) << i);
    }
    
    setCameraPosition(vec3(0.0f, 0.0f, 0.0f));
}

Terrain::~Terrain()
{

}

std::string Terrain::getVertexShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"uniform mat4 worldToScreen;\n"
"\n"
"in vec2 v_vertex;\n"
"in vec4 v_scaleAndTranslate;\n"
"\n"
"out vec3 f_worldPosition;\n"
"out float f_cameraDepth;\n"
"\n"
"void main(void)\n"
"{\n"
"   f_worldPosition = vec3(v_scaleAndTranslate.xy*v_vertex + v_scaleAndTranslate.zw, 0.0f).xzy;\n"
"   gl_Position = worldToScreen*vec4(f_worldPosition, 1.0f);\n"
"   f_cameraDepth = gl_Position.z;\n"
"}\n\0");
}

std::string Terrain::getFragmentShaderCode() const
{
    return std::string(
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D diffuseTexture;\n"
"\n"
"const float C = 1.0f, D = 1.0e6, E = 1.0f;\n"
"\n"
"in vec3 f_worldPosition;\n"
"in float f_cameraDepth;\n"
"\n"
"out vec4 diffuse;\n"
"out vec4 worldNormal;\n"
"out vec4 worldPosition;\n"
"\n"
"void main(void)\n"
"{\n"
"   diffuse = vec4(1.0f);\n"
"   worldNormal = vec4(0.0f, 1.0f, 0.0f, 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0");
}

void Terrain::updateBlockTranslations(const vec2 &viewer)
{
    ivec2 dir = ivec2(0, 0);
    
    //Determine whether or not we need to shift the blocks to recentre at the viewer's position.
    if (true)
    {
        int i;
        bool outOfBounds = false;
        
        for (i = maxLevel - 1; i >= 0 && !outOfBounds; --i)
        {
            const float left =  0.45f*(float)(superBlockSize << i);
            const float right = 0.55f*(float)(superBlockSize << i);
            
            if (viewer.x <= blockTranslations[i].x + left)
            {
                dir.x = -1;
                outOfBounds = true;
            }
            else if (viewer.x >= blockTranslations[i].x + right)
            {
                dir.x = 1;
                outOfBounds = true;
            }
            
            if (viewer.y <= blockTranslations[i].y + left)
            {
                dir.y = -1;
                outOfBounds = true;
            }
            else if (viewer.y >= blockTranslations[i].y + right)
            {
                dir.y = 1;
                outOfBounds = true;
            }
        }
        
        //If we are in the centre, do not update.
        if (!outOfBounds) return;
        
        minLevel = i + 1;
    }
    
    //Calculate new shift bits.
    ivec2 deltaShifts;
    
    if (true)
    {
        ivec2 newShifts = bitShifts;
        
        if (dir.x > 0)
        {
            if ((newShifts.x += (1 << minLevel)) >= (1 << (maxLevel - 1)))
            {
                newShifts.x -= (1 << (maxLevel - 1));
                
                for (int i = 0; i < maxLevel; i++) blockTranslations[i] += ivec2(1 << maxLevel, 0);
            }
        }
        else if (dir.x < 0)
        {
            if ((newShifts.x -= (1 << minLevel)) < 0)
            {
                newShifts.x += (1 << (maxLevel - 1));
                
                for (int i = 0; i < maxLevel; i++) blockTranslations[i] -= ivec2(1 << maxLevel, 0);
            }
        }
        
        if (dir.y > 0)
        {
            if ((newShifts.y += (1 << minLevel)) >= (1 << (maxLevel - 1)))
            {
                newShifts.y -= (1 << (maxLevel - 1));
                
                for (int i = 0; i < maxLevel; i++) blockTranslations[i] += ivec2(0, 1 << maxLevel);
            }
        }
        else if (dir.y < 0)
        {
            if ((newShifts.y -= (1 << minLevel)) < 0)
            {
                newShifts.y += (1 << (maxLevel - 1));
                
                for (int i = 0; i < maxLevel; i++) blockTranslations[i] -= ivec2(0, 1 << maxLevel);
            }
        }
        
        deltaShifts = ivec2(bitShifts.x ^ newShifts.x, bitShifts.y ^ newShifts.y);
        bitShifts = newShifts;
    }
    
    //Determine the size of the required shifts in the heightmap.
    int andMask = (1 << (maxLevel - 2));
    
    for (int i = maxLevel - 2; i >= 0; --i)
    {
        if ((deltaShifts.x & andMask) || (deltaShifts.y & andMask))
        {
            const ivec2 deltaDelta(
                        (deltaShifts.x & andMask) ? ((bitShifts.x & andMask) ? (andMask << 1) : -(andMask << 1)) : 0,
                        (deltaShifts.y & andMask) ? ((bitShifts.y & andMask) ? (andMask << 1) : -(andMask << 1)) : 0);
            
            for (int j = i; j >= 0; --j)
            {
                blockTranslations[j] += deltaDelta;
            }
        }
        
        andMask >>= 1;
    }
}

void Terrain::setCameraPosition(const vec3 &a_position)
{
    //Updates shifts and blockTranslations to re-centre the map at the player's position.
    updateBlockTranslations(vec2(a_position.x/scale.x, a_position.z/scale.z));
    
    //Create new instance buffers.
    smallBlock.instances[0] = TerrainBlockInstance(vec4(1.0f, 1.0f, 0.0f, 0.0f));
    smallBlock.instances.sendToDevice();
}

void Terrain::render(const ShaderProgram &program) const
{
    smallBlock.bind(program);
    renderIndicesAsTriangleStripsInstanced(smallBlock.indices, 1);
    smallBlock.unbind(program);
}

