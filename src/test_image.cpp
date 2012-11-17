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
#include <vector>
#include <cassert>
#include <climits>

#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <tiny/img/io/image.h>
#include <tiny/draw/computetexture.h>

//Default video settings.
int screenWidth = 800;
int screenHeight = 600;
int screenBPP = 0;
int screenDepthBPP = 24;
Uint32 screenFlags = 0;
SDL_Surface *screen = 0;

//Default image loading settings.
int imageLoaders = IMG_INIT_PNG | IMG_INIT_JPG;

//Camera.
bool cameraKeys[256] = {false};

tiny::draw::RGBATexture2D *testTexture = 0;
tiny::draw::ComputeTexture *computeTexture = 0;

using namespace std;

void setup()
{
    vector<string> inputTextures;
    vector<string> outputTextures;
    const string fragmentShader =
"#version 150\n"
"\n"
"precision highp float;\n"
"\n"
"uniform sampler2D source;\n"
"uniform vec2 sourceInverseSize;\n"
"\n"
"in vec2 tex;\n"
"out vec4 colour;\n"
"\n"
"void main(void)\n"
"{\n"
"   colour = texture(source, tex);\n"
"}\n";
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");
    
    computeTexture = new tiny::draw::ComputeTexture(inputTextures, outputTextures, fragmentShader);
    testTexture = new tiny::draw::RGBATexture2D(tiny::img::io::readImage("../test.png"));
    computeTexture->setInput(*testTexture, "source");
}

void cleanup()
{
    delete computeTexture;
    delete testTexture;
}

void update(const double &dt)
{

}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    computeTexture->compute();
}

int main(int argc, char **argv)
{
    //Initialise SDL.
    cerr << "Initialising SDL..." << endl;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        cerr << "Unable to initialise SDL: " << SDL_GetError() << "!" << endl;
        return -1;
    }
    
    //Make sure the colour depth is correct.
    if ((screenFlags & SDL_FULLSCREEN) != 0 || screenBPP < 8) screenBPP = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
    //Set depth buffer precision.
    if (screenDepthBPP > 0) SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, screenDepthBPP);
    //Enable double buffering.
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    //Create window.
    cerr << "Creating a " << screenWidth << "x" << screenHeight << " window at " << screenBPP << " bits per pixel..." << endl;
    
    screenFlags |= SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_HWPALETTE | SDL_HWSURFACE | SDL_HWACCEL;
    screen = SDL_SetVideoMode(screenWidth, screenHeight, screenBPP, screenFlags);
    
    if (!screen)
    {
        cerr << "Unable to create window: " << SDL_GetError() << "!" << endl;
        return -1;
    }
    
    SDL_EnableUNICODE(1);
    SDL_WM_SetCaption("", 0);
    
    //Initialise OpenGL and GLEW.
    cerr << "Initialising OpenGL..." << endl;
    
    if (glewInit() == GLEW_OK)
    {
        if (glGetString(GL_VERSION)) cerr << "Using OpenGL version " << glGetString(GL_VERSION) << "." << endl;
        else cerr << "Cannot determine OpenGL version!" << endl;

        if (glGetString(GL_SHADING_LANGUAGE_VERSION)) cerr << "Using GLSL version " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "." << endl;
        else cerr << "Cannot determine GLSL version!" << endl;

        //Check for necessary OpenGL extensions.
        if (!glGenBuffers ||
            !glBindBuffer ||
            !glBufferData ||
            !glDeleteBuffers ||
            !glUseProgram)
        {
            cerr << "Unable to find the necessary OpenGL extensions! Please make sure you are using OpenGL 3.2 compatible hardware!" << endl;

            SDL_Quit();
            return -1;
        }

        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClearDepth(1.0);
        
        glEnable(GL_DEPTH_TEST);    
        glDepthMask(GL_TRUE);
        
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndexNV(UINT_MAX);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        //Clear screen.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapBuffers();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapBuffers();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapBuffers();
    }
    else
    {
        cerr << "Unable to initialise GLEW!" << endl;
        return -1;
    }
    
    //Initialise image loading.
    cerr << "Initialising image loading..." << endl;
    
    if (IMG_Init(imageLoaders) != imageLoaders)
    {
        cerr << "Unable to initialise SDL_image: " << IMG_GetError() << "!" << endl;
        return -1;
    }
    
    //Initialise font loading.
    cerr << "Initialising font loading..." << endl;
    
    if (TTF_Init() != 0)
    {
        cerr << "Unable to initialise SDL_ttf: " << TTF_GetError() << "!" << endl;
        return -1;
    }
    
    //Setup.
    try
    {
        setup();
    }
    catch (std::exception &e)
    {
        cerr << "Unable to setup game!" << endl;
        return -1;
    }
    
    //Start main loop.
    cerr << "Initialisation complete." << endl;
    
    bool running = true;
    bool wireframe = false;
    Uint32 lastCount = SDL_GetTicks(), curCount = SDL_GetTicks();
    
    while (running)
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
                    running = false;
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
                    cameraKeys[k] = true;
                }
            }
            else if (event.type == SDL_KEYUP)
            {
                const SDLKey k = event.key.keysym.sym;
                
                if (k >= 0 && k < 256)
                {
                    cameraKeys[k] = false;
                }
            }
            else if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }
        
        //Find out the update time.
        const double dt = 1.0e-3*static_cast<double>((curCount = SDL_GetTicks()) - lastCount);
        
        lastCount = curCount;
        
        //Update screen.
        update(dt);
        render();
        SDL_GL_SwapBuffers();
        SDL_Delay(10);
    }
    
    //Shut down everything.
    cerr << "Shutting down..." << endl;
    
    cleanup();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

