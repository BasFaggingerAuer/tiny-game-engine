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
#include <climits>

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <tiny/os/sdlapplication.h>

using namespace tiny::os;

SDLApplication::SDLApplication(const int &a_screenWidth,
                               const int &a_screenHeight,
                               const int &a_screenBPP,
                               const int &a_screenDepthBPP) :
    Application(),
    screenWidth(a_screenWidth),
    screenHeight(a_screenHeight),
    screenBPP(a_screenBPP),
    screenDepthBPP(a_screenDepthBPP),
    screenFlags(0),
    screen(0),
    wireframe(false)
{
    //Initialise SDL.
    std::cerr << "Initialising SDL..." << std::endl;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        std::cerr << "Unable to initialise SDL: " << SDL_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Make sure the colour depth is correct.
    if ((screenFlags & SDL_FULLSCREEN) != 0 || screenBPP < 8) screenBPP = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
    //Set depth buffer precision.
    if (screenDepthBPP > 0) SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, screenDepthBPP);
    //Enable double buffering.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    //Create window.
    std::cerr << "Creating a " << screenWidth << "x" << screenHeight << " window at " << screenBPP << " bits per pixel..." << std::endl;
    
    screenFlags |= SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_HWSURFACE | SDL_HWACCEL;
    screen = SDL_SetVideoMode(screenWidth, screenHeight, screenBPP, screenFlags);
    
    if (!screen)
    {
        std::cerr << "Unable to create window: " << SDL_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption("", 0);
    
    //Initialise OpenGL and GLEW.
    std::cerr << "Initialising OpenGL..." << std::endl;
    
    if (glewInit() == GLEW_OK)
    {
        initOpenGL();
    }
    else
    {
        std::cerr << "Unable to initialise GLEW!" << std::endl;
        throw std::exception();
    }
    
    //Clear screen.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapBuffers();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapBuffers();
    
    //Initialise image loading.
    std::cerr << "Initialising image loading..." << std::endl;
    
    if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) != (IMG_INIT_PNG | IMG_INIT_JPG))
    {
        std::cerr << "Unable to initialise SDL_image: " << IMG_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Initialise font loading.
    std::cerr << "Initialising font loading..." << std::endl;
    
    if (TTF_Init() != 0)
    {
        std::cerr << "Unable to initialise SDL_ttf: " << TTF_GetError() << "!" << std::endl;
        throw std::exception();
    }
    
    //Start main loop.
    std::cerr << "Initialisation complete." << std::endl;
    
    lastCount = SDL_GetTicks();
    curCount = SDL_GetTicks();
}

SDLApplication::~SDLApplication()
{
    //Shut down everything.
    std::cerr << "Shutting down SDL..." << std::endl;
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void SDLApplication::initOpenGL()
{
    if (glGetString(GL_VERSION)) std::cerr << "Using OpenGL version " << glGetString(GL_VERSION) << "." << std::endl;
    else std::cerr << "Cannot determine OpenGL version!" << std::endl;

    if (glGetString(GL_SHADING_LANGUAGE_VERSION)) std::cerr << "Using GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << std::endl;
    else std::cerr << "Cannot determine GLSL version!" << std::endl;

    //Check for necessary OpenGL extensions.
    if (!glGenBuffers ||
        !glBindBuffer ||
        !glBufferData ||
        !glDeleteBuffers ||
        !glUseProgram)
    {
        std::cerr << "Unable to find the necessary OpenGL extensions! Please make sure you are using OpenGL 3.2 compatible hardware!" << std::endl;

        SDL_Quit();
        throw std::exception();
    }

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0);
    
    glEnable(GL_DEPTH_TEST);    
    glDepthMask(GL_TRUE);
    
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndexNV(UINT_MAX);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

double SDLApplication::pollEvents()
{
    //Poll pending events.
    SDL_Event event;
    
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_KEYDOWN)
        {
            const SDLKey k = event.key.keysym.sym;
            
            if (k == SDLK_ESCAPE)
            {
                stopRunning();
            }
            else if (k == SDLK_F1)
            {
                //Enable wireframe view.
                wireframe = !wireframe;
                
                if (wireframe)
                {
                    glPolygonMode(GL_FRONT, GL_LINE);
                    glPolygonMode(GL_BACK, GL_LINE);
                }
                else
                {
                    glPolygonMode(GL_FRONT, GL_FILL);
                    glPolygonMode(GL_BACK, GL_FILL);
                }
            }
            else if (k == SDLK_PRINT)
            {
                //Save a screenshot.
                SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, screenWidth, screenHeight, 24, 255 << 0, 255 << 8, 255 << 16, 0);

                if (image)
                {
                    glFlush();
                    glReadBuffer(GL_BACK);
                    glReadPixels(0, 0, screenWidth, screenHeight, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
                    SDL_SaveBMP(image, "screenshot.bmp");
                    SDL_FreeSurface(image);
                }
            }
            else if (k >= 0 && k < 256)
            {
                pressedKeys[k] = true;
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            const SDLKey k = event.key.keysym.sym;
            
            if (k >= 0 && k < 256)
            {
                pressedKeys[k] = false;
            }
        }
        else if (event.type == SDL_QUIT)
        {
            stopRunning();
        }
    }
    
    //Find out the update time.
    const double dt = 1.0e-3*static_cast<double>((curCount = SDL_GetTicks()) - lastCount);
    
    lastCount = curCount;
    
    return dt;
}

void SDLApplication::paint()
{
    SDL_GL_SwapBuffers();
    SDL_Delay(10);
}

int SDLApplication::getScreenWidth() const
{
    return screenWidth;
}

int SDLApplication::getScreenHeight() const
{
    return screenHeight;
}

