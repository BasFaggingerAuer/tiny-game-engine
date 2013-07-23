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

#include <tiny/img/image.h>

#include <tiny/draw/iconhorde.h>
#include <tiny/draw/effects/diffuse.h>
#include <tiny/draw/worldrenderer.h>

using namespace std;
using namespace tiny;

os::Application *application = 0;

draw::WorldRenderer *worldRenderer = 0;

std::vector<draw::WorldIconInstance> iconInstances;
draw::WorldIconHorde *iconHorde = 0;
draw::RGBATexture2D *iconIconTexture = 0;

draw::Renderable *screenEffect = 0;

vec3 cameraPosition = vec3(0.0f, 0.0f, 10.0f);
vec4 cameraOrientation = vec4(0.0f, 0.0f, 0.0f, 1.0f);

void setup()
{
    //Create a icon mesh and paint it with a texture.
    iconHorde = new draw::WorldIconHorde(1024);
    iconIconTexture = new draw::RGBATexture2D(img::Image::createTestImage());
    iconHorde->setIconTexture(*iconIconTexture);
    
    //Create instances of the icons in a grid.
    const float iconSpacing = 3.0f;
    
    for (int i = -4; i <= 4; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            for (int k = -4; k <= 4; ++k)
            {
                iconInstances.push_back(draw::WorldIconInstance(vec4(iconSpacing*i, iconSpacing*j, iconSpacing*k, 1.0f), vec2(1.0f, 1.0f), vec4(0.0f, 0.0f, 1.0f, 1.0f), vec4(1.0f, 1.0f, 1.0f, 1.0f)));
            }
        }
    }
    
    iconHorde->setIcons(iconInstances.begin(), iconInstances.end());
    
    //Render only diffuse colours to the screen.
    screenEffect = new draw::effects::Diffuse();
    
    //Create a renderer and add the icon and the diffuse rendering effect to it.
    worldRenderer = new draw::WorldRenderer(application->getScreenWidth(), application->getScreenHeight());
    worldRenderer->addWorldRenderable(iconHorde);
    worldRenderer->addScreenRenderable(screenEffect, false, false);
}

void cleanup()
{
    delete worldRenderer;
    
    delete screenEffect;
    
    delete iconHorde;
    delete iconIconTexture;
}

void update(const double &dt)
{
    //Move the camera around.
    application->updateSimpleCamera(dt, cameraPosition, cameraOrientation);
    
    //Tell the world renderer that the camera has changed.
    worldRenderer->setCamera(cameraPosition, cameraOrientation);
}

void render()
{
    worldRenderer->clearTargets();
    worldRenderer->render();
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

