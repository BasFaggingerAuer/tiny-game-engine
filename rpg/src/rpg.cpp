/*
Copyright 2020, Bas Fagginger Auer.

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
#include <map>
#include <string>
#include <exception>

#include <tiny/os/application.h>
#include <tiny/os/sdlapplication.h>

#include "messages.h"
#include "network.h"
#include "game.h"

int main(int argc, char **argv)
{
    tiny::os::Application *application = 0;
    rpg::Game *game = 0;
    
    try
    {
        int screenWidth = 1600;
        int screenHeight = 900;
        std::string basePath = "";
        
        if (argc != 1 && argc != 2 && argc != 4)
        {
            std::cerr << "Usage: " << argv[0] << " [path/to/resources/] [screenwidth screenheight]" << std::endl;
            throw std::exception();
        }

        if (argc >= 2)
        {
            basePath = std::string(argv[1]);
        }
        
        if (argc >= 4)
        {
            screenWidth = atoi(argv[2]);
            screenHeight = atoi(argv[3]);
        }
        
        application = new tiny::os::SDLApplication(screenWidth, screenHeight);

        game = new rpg::Game(application, basePath);
    }
    catch (std::exception &)
    {
        std::cerr << "Unable to start application!" << std::endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        game->update(application, static_cast<float>(application->pollEvents()));
        game->render();
        application->paint();
    }
    
    delete game;
    delete application;
    
    std::cerr << "Goodbye." << std::endl;
    
    return 0;
}

