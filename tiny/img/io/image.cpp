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
#include <iostream>
#include <exception>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <tiny/img/io/image.h>

using namespace tiny::img;

Image tiny::img::io::readImage(const std::string &fileName)
{
    //Read image from disk.
    Image image;
    SDL_Surface *surface = IMG_Load(fileName.c_str());
    
    if (!surface)
    {
        std::cerr << "Unable to read '" << fileName << "': " << IMG_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    if (surface->w <= 0 || surface->h <= 0)
    {
        std::cerr << "Image '" << fileName << "' contains no data!" << std::endl;
        throw std::exception();
    }
    
    image = Image(surface->w, surface->h);
    
    //Gather pixel data.
    unsigned char *destination = &image.data[0];
    const unsigned char *srcRow = static_cast<unsigned char *>(surface->pixels);
    const size_t nrChannels = surface->format->BytesPerPixel;
    
    for (size_t y = 0; y < image.height; ++y)
    {
        const unsigned char *src = srcRow;
        
        for (size_t x = 0; x < image.width; ++x)
        {
            size_t i = 0;
            
            for (i = 0; i < 4 && i < nrChannels; ++i)
            {
                *destination++ = *src++;
            }
            
            for ( ; i < 4; ++i)
            {
                *destination++ = 255;
            }
        }
        
        srcRow += surface->pitch;
    }
    
    SDL_FreeSurface(surface);
    
    std::cerr << "Read a " << image.width << "x" << image.height << " image from '" << fileName << "' with " << nrChannels << " channels." << std::endl;
    
    return image;
}

std::vector<Image> tiny::img::io::readFont(const std::string &fileName, const int &fontSize)
{
    std::vector<Image> images;
    TTF_Font *font = TTF_OpenFont(fileName.c_str(), fontSize);
    char text[2] = {' ', '\0'};
    SDL_Color color;
    
    color.r = 255; color.g = 255; color.b = 255;
    images.reserve(256);
    
    if (!font)
    {
        std::cerr << "Unable to read '" << fileName << "' as font: " << TTF_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    for (int i = 0; i < 128; ++i)
    {
        text[0] = static_cast<char>(i);
        SDL_Surface *data = TTF_RenderText_Blended(font, text, color);
        Image image = Image::createTestImage(8);
        
        if (!data)
        {
            //cerr << "Unable to render character " << i << "!" << endl;
        }
        else if (data->w <= 0 || data->h <= 0 || data->format->BytesPerPixel != 4)
        {
            std::cerr << "Empty character data or invalid number of bytes per pixel for " << i << "!" << std::endl;
        }
        else
        {
            //Create image from the data.
            image = Image(data->w, data->h);
            
            unsigned char *srcPointer = static_cast<unsigned char *>(data->pixels);
            unsigned char *destPointer = &image.data[0];
            
            for (size_t y = 0; y < image.height; ++y)
            {
                for (size_t x = 0; x < image.width; ++x)
                {
                    *destPointer++ = srcPointer[4*x + 0];
                    *destPointer++ = srcPointer[4*x + 1];
                    *destPointer++ = srcPointer[4*x + 2];
                    *destPointer++ = srcPointer[4*x + 3];
                }
                
                srcPointer += data->pitch;
            }
            
            SDL_FreeSurface(data);
        }
        
        images.push_back(image);
    }
    
    TTF_CloseFont(font);
    
    std::cerr << "Read a font from '" << fileName << "'." << std::endl;
    
    return images;
}


