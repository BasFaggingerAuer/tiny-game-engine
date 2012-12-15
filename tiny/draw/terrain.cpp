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

TerrainBlockInstanceBufferInterpreter::TerrainBlockInstanceBufferInterpreter(const size_t &maxNrInstances) :
    VertexBufferInterpreter<TerrainBlockInstance>(maxNrInstances)
{
    addVec4Attribute(0*sizeof(float), "v_scaleAndTranslate");
}

TerrainBlockInstanceBufferInterpreter::~TerrainBlockInstanceBufferInterpreter()
{

}

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

void TerrainBlock::sendToDevice() const
{
    instances.sendToDevice();
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

TerrainStitchVertexBufferInterpreter::TerrainStitchVertexBufferInterpreter(const size_t &width) :
    VertexBufferInterpreter<vec2>(4*3*(width - 1))
{
    size_t count = 0;
    
    for (size_t i = 0; i < width;       ++i) hostData[count++] = vec2(2*i, 0);
    for (size_t i = 0; i < 2*width - 3; ++i) hostData[count++] = vec2(1 + i, 1);
    
    for (size_t i = 0; i < width;       ++i) hostData[count++] = vec2(0, 2*(width - 1 - i));
    for (size_t i = 0; i < 2*width - 3; ++i) hostData[count++] = vec2(1, 1 + 2*width - 4 - i);
    
    for (size_t i = 0; i < width;       ++i) hostData[count++] = vec2(2*(width - 1 - i), 2*width - 2);
    for (size_t i = 0; i < 2*width - 3; ++i) hostData[count++] = vec2(1 + 2*width - 4 - i, 2*width - 3);
    
    for (size_t i = 0; i < width;       ++i) hostData[count++] = vec2(2*width - 2, 2*i);
    for (size_t i = 0; i < 2*width - 3; ++i) hostData[count++] = vec2(2*width - 3, 1 + i);
    
    sendToDevice();
    
    addVec2Attribute(0, "v_vertex");
}

TerrainStitchVertexBufferInterpreter::~TerrainStitchVertexBufferInterpreter()
{
    
}

TerrainStitchIndexBuffer::TerrainStitchIndexBuffer(const size_t &width) :
    IndexBuffer<unsigned int>(4*(8 + 4*(width - 3)))
{
    size_t count = 0;
    
    for (size_t i = 0; i < 4; ++i)
    {
        const size_t offset = 3*i*(width - 1);
        
        hostData[count++] = offset;
        hostData[count++] = offset + width;
        
        for (size_t j = 0; j < width - 3; ++j)
        {
            hostData[count++] = offset + 1 + j;
            hostData[count++] = offset + width + 1 + 2*j;
            hostData[count++] = offset + 1 + j;
            hostData[count++] = offset + width + 2 + 2*j;
        }
        
        hostData[count++] = offset + width - 2;
        hostData[count++] = offset + 3*width - 5;
        hostData[count++] = offset + width - 2;
        hostData[count++] = offset + 3*width - 4;
        hostData[count++] = offset + width - 1;
        hostData[count++] = UINT_MAX;
    }
    
    sendToDevice();
}

TerrainStitchIndexBuffer::~TerrainStitchIndexBuffer()
{

}

TerrainStitch::TerrainStitch(const size_t &size, const size_t &maxNrInstances) :
    vertices(size),
    instances(maxNrInstances),
    indices(size)
{

}

TerrainStitch::~TerrainStitch()
{

}

TerrainBlockInstance &TerrainStitch::operator [] (const size_t &index)
{
    return instances[index];
}

const TerrainBlockInstance &TerrainStitch::operator [] (const size_t &index) const
{
    return instances[index];
}

void TerrainStitch::sendToDevice() const
{
    instances.sendToDevice();
}

void TerrainStitch::bind(const ShaderProgram &program) const
{
    vertices.bind(program);
    instances.bind(program, 1);
}

void TerrainStitch::unbind(const ShaderProgram &program) const
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
    crossBlockY(5, blockSize, 2*maxLevel),
    ellBlockX(2*blockSize + 3, 2, maxLevel),
    ellBlockY(2, 2*blockSize + 3, maxLevel),
    stitch(2*blockSize + 2, maxLevel),
    nrSmallBlocks(0),
    nrCrossBlocks(0),
    nrLargeBlocks(0),
    nrEllBlocks(0),
    nrStitch(0)
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
"   f_worldPosition.y = 16.0f*sin(0.1f*f_worldPosition.x)*sin(0.1f*f_worldPosition.z)*(1.0f - exp(-0.01f*length(f_worldPosition.xz)));\n"
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
"   diffuse = vec4(0.5f, 0.5f, 0.5f, 1.0f);\n"
"   worldNormal = vec4(0.0f, 1.0f, 0.0f, 0.0f);\n"
"   worldPosition = vec4(f_worldPosition, f_cameraDepth);\n"
"   \n"
"   gl_FragDepth = (log(C*f_cameraDepth + E) / log(C*D + E));\n"
"}\n\0");
}

bool Terrain::updateBlockTranslations(const vec2 &viewer)
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
        if (!outOfBounds) return false;
        
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
    
    return true;
}

void Terrain::setCameraPosition(const vec3 &a_position)
{
    //Updates shifts and blockTranslations to re-centre the map at the player's position.
    //Do not update display lists if the camera's position has not changed.
    if (!updateBlockTranslations(vec2(a_position.x/scale.x, a_position.z/scale.z))) return;
    
    //Create new instance buffers.
    nrSmallBlocks = 0;
    nrCrossBlocks = 0;
    nrLargeBlocks = 0;
    nrEllBlocks = 0;
    nrStitch = 0;
    
    for (int i = minLevel; i < maxLevel; ++i)
    {
        //Calculate scale factor and set up translations.
        const vec2 r = (float)(1 << i)*vec2(scale.x, scale.z);
        const vec2 s = (float)(4 << i)*vec2(scale.x, scale.z);
        const vec2 bs = (float)((blockSize - 1) << i)*vec2(scale.x, scale.z);
        const vec2 t = vec2(blockTranslations[i].x*scale.x, blockTranslations[i].y*scale.z);
        
        //Draw blocks.
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 3.0f*bs.x, t.y + s.y + 2.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 3.0f*bs.x, t.y + s.y + 3.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 2.0f*bs.x, t.y + s.y + 3.0f*bs.y));

        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 1.0f*bs.x, t.y + s.y + 3.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 0.0f*bs.x, t.y + s.y + 3.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 0.0f*bs.x, t.y + s.y + 2.0f*bs.y));
        
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 0.0f*bs.x, t.y + 1.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 0.0f*bs.x, t.y + 0.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 1.0f*bs.x, t.y + 0.0f*bs.y));
        
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 2.0f*bs.x, t.y + 0.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 3.0f*bs.x, t.y + 0.0f*bs.y));
        smallBlock[nrSmallBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 3.0f*bs.x, t.y + 1.0f*bs.y));
		
        //Draw cross.
        crossBlockX[nrCrossBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x, t.y + 2.0f*bs.y));
        crossBlockY[nrCrossBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 2.0f*bs.x, t.y));
        nrCrossBlocks++;
        crossBlockX[nrCrossBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x + s.x + 3.0f*bs.x, t.y + 2.0f*bs.y));
        crossBlockY[nrCrossBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x + 2.0f*bs.x, t.y + s.y + 3.0f*bs.y));
        nrCrossBlocks++;
        
        if (i == minLevel)
        {
            //Draw big block in the centre.
            largeBlock[nrLargeBlocks++] = TerrainBlockInstance(vec4(r.x, r.y, t.x + bs.x, t.y + bs.y));
        }
        else
        {
            //Draw L.
            ellBlockX[nrEllBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x + bs.x, t.y + bs.y + ((bitShifts.y & (1 << (i - 1))) != 0 ? 0.0f : 2.0f*bs.y + s.y - r.y)));
            ellBlockY[nrEllBlocks] = TerrainBlockInstance(vec4(r.x, r.y, t.x + bs.x + ((bitShifts.x & (1 << (i - 1))) != 0 ? 0.0f : 2.0f*bs.x + s.x - r.x), t.y + bs.y + ((bitShifts.y & (1 << (i - 1))) != 0 ? r.y : 0.0f)));
            nrEllBlocks++;
        }
        
        //Draw stitch.
        stitch[nrStitch++] = TerrainBlockInstance(vec4(r.x, r.y, t.x - r.x, t.y - r.y));
    }
    
    //Update buffers on the GPU.
    smallBlock.sendToDevice();
    crossBlockX.sendToDevice();
    crossBlockY.sendToDevice();
    largeBlock.sendToDevice();
    ellBlockX.sendToDevice();
    ellBlockY.sendToDevice();
    stitch.sendToDevice();
}

void Terrain::render(const ShaderProgram &program) const
{
    largeBlock.bind(program);
    renderIndicesAsTriangleStripsInstanced(largeBlock.indices, nrLargeBlocks);
    largeBlock.unbind(program);
    
    smallBlock.bind(program);
    renderIndicesAsTriangleStripsInstanced(smallBlock.indices, nrSmallBlocks);
    smallBlock.unbind(program);
    
    crossBlockX.bind(program);
    renderIndicesAsTriangleStripsInstanced(crossBlockX.indices, nrCrossBlocks);
    crossBlockX.unbind(program);
    
    crossBlockY.bind(program);
    renderIndicesAsTriangleStripsInstanced(crossBlockY.indices, nrCrossBlocks);
    crossBlockY.unbind(program);
    
    ellBlockX.bind(program);
    renderIndicesAsTriangleStripsInstanced(ellBlockX.indices, nrEllBlocks);
    ellBlockX.unbind(program);
    
    ellBlockY.bind(program);
    renderIndicesAsTriangleStripsInstanced(ellBlockY.indices, nrEllBlocks);
    ellBlockY.unbind(program);
    
    stitch.bind(program);
    renderIndicesAsTriangleStripsInstanced(stitch.indices, nrStitch);
    stitch.unbind(program);
}

