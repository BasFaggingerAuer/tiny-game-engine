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
#include <tiny/os/application.h>

using namespace tiny::os;

Application::Application() :
    running(true)
{
    for (int i = 0; i < 256; ++i) pressedKeys[i] = false;
}

Application::~Application()
{

}

bool Application::isRunning() const
{
    return running;
}

bool Application::isKeyPressed(const int &key) const
{
    if (key < 0 || key >= 256) return false;
    
    return pressedKeys[key];
}

void Application::stopRunning()
{
    running = false;
}

