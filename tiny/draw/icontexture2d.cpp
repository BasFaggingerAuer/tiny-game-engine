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

#include <tiny/draw/icontexture2d.h>

using namespace tiny;
using namespace tiny::draw;

IconTexture2D::IconTexture2D(const size_t &a_width, const size_t &a_height) :
    Texture2D<unsigned char, 4>(a_width, a_height)
{
    clearIcons();
}

IconTexture2D::~IconTexture2D()
{

}

void IconTexture2D::clearIcons()
{
    //Start with a big free cell.
    maxSubImageDimensions = ivec2(0, 0);
    
    xBounds.clear();
    xBounds.push_back(0);
    xBounds.push_back(width);
    yBounds.clear();
    yBounds.push_back(0);
    yBounds.push_back(height);
    
    occupied.clear();
    occupied[ivec2(0, 0)] = false;
    occupied[ivec2(width, 0)] = true;
    occupied[ivec2(0, height)] = true;
    occupied[ivec2(width, height)] = true;
    
    subImages.clear();
}

vec4 IconTexture2D::addSingleIcon(const img::Image &image)
{
    vec4 finalPosition = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    bool found = false;
    
    //Find unoccupied cells to store the image.
    for (std::list<int>::iterator x0 = xBounds.begin(); x0 != xBounds.end() && !found; ++x0)
    {
        for (std::list<int>::iterator y0 = yBounds.begin(); y0 != yBounds.end() && !found; ++y0)
        {
            const ivec2 pos0(*x0, *y0);
            
            assert(occupied.find(pos0) != occupied.end());
            
            if (!occupied[pos0])
            {
                //This cell is free, start expanding it until it encompasses the image.
                std::list<int>::iterator x1 = x0, y1 = y0;
                
                while (x1 != xBounds.end())
                {
                    if (*x1 - *x0 < (int)image.width) ++x1;
                    else break;
                }
                
                while (y1 != yBounds.end())
                {
                    if (*y1 - *y0 < (int)image.height) ++y1;
                    else break;
                }
                
                if (x1 != xBounds.end() && y1 != yBounds.end())
                {
                    bool blocked = false;
                    
                    //Verify that all encountered cells are not occupied.
                    for (std::list<int>::iterator x2 = x0; x2 != x1 && !blocked; ++x2)
                    {
                        for (std::list<int>::iterator y2 = y0; y2 != y1 && !blocked; ++y2)
                        {
                            const ivec2 pos2(*x2, *y2);
                            
                            assert(occupied.find(pos2) != occupied.end());
                            
                            if (occupied[pos2]) blocked = true;
                        }
                    }
                    
                    if (!blocked)
                    {
                        //We found a spot where we can put our image.
                        found = true;
                        
                        //Store it.
                        subImages.push_back(ivec4(pos0.x, pos0.y, image.width, image.height));
                        finalPosition = vec4((float)pos0.x/(float)width,
                                    (float)pos0.y/(float)height,
                                    (float)image.width/(float)width,
                                    (float)image.height/(float)height);
                        
                        //Split cells if required.
                        if (*x1 - *x0 > (int)image.width)
                        {
                            const int xNew = *x0 + image.width;
                            
                            xBounds.insert(x1, xNew);
                            --x1;
                            
                            std::list<int>::iterator x2 = x1;
                            
                            --x2;
                            assert(*x1 == xNew);
                            assert(*x2 < xNew);
                            
                            for (std::list<int>::iterator y2 = yBounds.begin(); y2 != yBounds.end(); ++y2) occupied[ivec2(*x1, *y2)] = occupied[ivec2(*x2, *y2)];
                        }
                        
                        if (*y1 - *y0 > (int)image.height)
                        {
                            const int yNew = *y0 + image.height;
                            
                            yBounds.insert(y1, yNew);
                            --y1;
                            
                            std::list<int>::iterator y2 = y1;
                            
                            --y2;
                            assert(*y1 == yNew);
                            assert(*y2 < yNew);
                            
                            for (std::list<int>::iterator x2 = xBounds.begin(); x2 != xBounds.end(); ++x2) occupied[ivec2(*x2, *y1)] = occupied[ivec2(*x2, *y2)];
                        }
                        
                        //Mark all occupied cells.
                        for (std::list<int>::iterator x2 = x0; x2 != x1; ++x2)
                        {
                            for (std::list<int>::iterator y2 = y0; y2 != y1; ++y2)
                            {
                                occupied[ivec2(*x2, *y2)] = true;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if (!found)
    {
        std::cerr << "Unable to pack an " << image.width << "x" << image.height << "image into a " << width << "x" << height << " icon texture!" << std::endl;
        return finalPosition;
    }
    
    //Blit image.
    const ivec4 pos = subImages.back();
    const unsigned char *srcPointer = &image.data[0];
    unsigned char *destPointer = &hostData[4*(pos.x + width*pos.y)];
    
    assert(pos.z == (int)image.width && pos.w == (int)image.height);
    
    maxSubImageDimensions.x = std::max(maxSubImageDimensions.x, pos.z);
    maxSubImageDimensions.y = std::max(maxSubImageDimensions.y, pos.w);
    
    for (int y = 0; y < pos.w; ++y)
    {
        std::copy(srcPointer, srcPointer + 4*pos.z, destPointer);
        srcPointer += 4*pos.z;
        destPointer += 4*width;
    }
    
    return finalPosition;
}

vec4 IconTexture2D::packIcon(const img::Image &image)
{
    const vec4 finalPosition = addSingleIcon(image);
    
    sendToDevice();
    
    std::cerr << "Packed a " << image.width << "x" << image.height << " icon into a " << width << "x" << height << " icon texture." << std::endl;
    
    return finalPosition;
}

void IconTexture2D::packIcons(const std::vector<img::Image> &images)
{
    for (std::vector<img::Image>::const_iterator i = images.begin(); i != images.end(); ++i)
    {
        addSingleIcon(*i);
    }
    
    std::cerr << "Packed " << images.size() << " icons into a " << width << "x" << height << " icon texture." << std::endl;
    
    sendToDevice();
}

vec4 IconTexture2D::getIcon(const int &index) const
{
    if (index < 0 || index >= static_cast<int>(subImages.size())) return vec4(0.0f, 0.0f, 1.0f, 1.0f);
    
    return vec4((float)subImages[index].x/(float)width,
                (float)subImages[index].y/(float)height,
                (float)subImages[index].z/(float)width,
                (float)subImages[index].w/(float)height);
}

vec2 IconTexture2D::getMaxIconDimensions() const
{
    return vec2((float)maxSubImageDimensions.x/(float)width,
                (float)maxSubImageDimensions.y/(float)height);
}

