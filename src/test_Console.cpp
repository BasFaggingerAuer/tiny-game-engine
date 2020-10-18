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
#include <string>
#include <exception>

#include <config.h>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include <tiny/img/io/image.h>

#include <tiny/draw/renderer.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/net/console.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::Renderer *renderer = 0;

double aspectRatio = 1.0;
draw::ScreenIconHorde *font = 0;
draw::IconTexture2D *fontTexture = 0;
net::Console *console = 0;

void setup()
{
    aspectRatio = static_cast<double>(application->getScreenWidth())/static_cast<double>(application->getScreenHeight());
    
    //Read font from disk as a texture.
    fontTexture = new draw::IconTexture2D(512, 512);
    fontTexture->packIcons(img::io::readFont(DATA_DIRECTORY + "font/OpenBaskerville-0.0.75.ttf", 48));
    
    //Create a drawable font object as a collection of instanced screen icons.
    font = new draw::ScreenIconHorde(1024);
    font->setIconTexture(*fontTexture);
    
    //Create a console.
    console = new net::Console();
    console->addLine("Please enter console commands below.");
    font->setText(-1.0, -1.0, 0.1, aspectRatio, console->getText(1), *fontTexture);
    
    //Create a renderer and add the font to it, disabling depth reading and writing.
    renderer = new draw::Renderer(true);
    renderer->addRenderable(0, font, false, false, draw::BlendMix);
}

void cleanup()
{
    delete renderer;
    
    delete console;
    delete font;
    delete fontTexture;
}

void update(const double &)
{
    bool updateConsole = false;
    
    for (int i = 0; i < 256; ++i)
    {
        if (application->isKeyPressedOnce(i))
        {
            updateConsole = true;
            console->keyDown(i);
        }
    }
    
    if (updateConsole)
    {
        font->setText(-1.0, -1.0, 0.1, aspectRatio, console->getText(32), *fontTexture);
    }
}

void render()
{
    renderer->clearTargets();
    renderer->render();
}

int main(int, char **)
{
    try
    {
        application = new os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        setup();
    }
    catch (std::exception &e)
    {
        cerr << "Unable to start application!" << endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        update(application->pollEvents());
        render();
        application->paint();
    }
    
    cleanup();
    delete application;
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

