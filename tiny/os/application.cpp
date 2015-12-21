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

bool Application::isKeyPressedOnce(const int &key)
{
    if (key < 0 || key >= 256) return false;
    
    const bool pressed = pressedKeys[key];
    
    pressedKeys[key] = false;
    
    return pressed;
}

void Application::keyDownCallback(const int &)
{
    
}

void Application::keyUpCallback(const int &)
{
    
}

void Application::stopRunning()
{
    running = false;
}

void Application::updateSimpleCamera(const float &dt, vec3 &cameraPosition, vec4 &cameraOrientation) const
{
    //Update the position and orientation of a simple controllable camera.
    const float ds = (isKeyPressed('f') ? 300.0f : 2.0f)*dt;
    const float dr = 2.1f*dt;
    
    if (isKeyPressed('i')) cameraOrientation = quatmul(quatrot(dr, vec3(-1.0f, 0.0f, 0.0f)), cameraOrientation);
    if (isKeyPressed('k')) cameraOrientation = quatmul(quatrot(dr, vec3( 1.0f, 0.0f, 0.0f)), cameraOrientation);
    if (isKeyPressed('j')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f,-1.0f, 0.0f)), cameraOrientation);
    if (isKeyPressed('l')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 1.0f, 0.0f)), cameraOrientation);
    if (isKeyPressed('u')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f,-1.0f)), cameraOrientation);
    if (isKeyPressed('o')) cameraOrientation = quatmul(quatrot(dr, vec3( 0.0f, 0.0f, 1.0f)), cameraOrientation);

    cameraOrientation = normalize(cameraOrientation);

    vec3 vel = mat4(cameraOrientation)*vec3((isKeyPressed('d') && isKeyPressed('a')) ? 0.0f : (isKeyPressed('d') ? 1.0f : (isKeyPressed('a') ? -1.0f : 0.0f)),
                                            (isKeyPressed('q') && isKeyPressed('e')) ? 0.0f : (isKeyPressed('q') ? 1.0f : (isKeyPressed('e') ? -1.0f : 0.0f)),
                                            (isKeyPressed('s') && isKeyPressed('w')) ? 0.0f : (isKeyPressed('s') ? 1.0f : (isKeyPressed('w') ? -1.0f : 0.0f)));
    
    cameraPosition += ds*normalize(vel);
}

