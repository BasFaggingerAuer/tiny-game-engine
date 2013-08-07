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
#include <tiny/net/message.h>
#include <tiny/net/host.h>
#include <tiny/net/client.h>

tiny::os::Application *application = 0;
tiny::net::Client *client = 0;
tiny::net::Host *host = 0;

using namespace std;

void cleanup()
{
    if (host) delete host;
    if (client) delete client;
}

void render()
{
    if (host) host->listen(0.25);
    if (client) client->listen(0.25);
}

int main(int argc, char **argv)
{
    try
    {
        application = new tiny::os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        if (argc == 2)
        {
            host = new tiny::net::Host(atoi(argv[1]));
        }
        else if (argc == 3)
        {
            client = new tiny::net::Client(argv[1], atoi(argv[2]));
        }
        else
        {
            cerr << "Provide either one or two command line arguments to act either as host or as client!" << std::endl;
            throw std::exception();
        }
    }
    catch (std::exception &e)
    {
        cerr << "Unable to start application!" << endl;
        return -1;
    }
    
    while (application->isRunning())
    {
        application->pollEvents();
        render();
        application->paint();
    }
    
    cleanup();
    delete application;
    
    cerr << "Goodbye." << endl;
    
    return 0;
}

