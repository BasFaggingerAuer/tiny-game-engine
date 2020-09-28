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
#pragma once

#include <string>

#include <tiny/math/vec.h>

namespace tiny
{

namespace os
{

struct MouseState
{
    MouseState() :
        x(0.0f),
        y(0.0f),
        buttons(0)
    {

    }
    
    MouseState(const float &a_x,
               const float &a_y,
               const unsigned int &a_buttons) :
        x(a_x),
        y(a_y),
        buttons(a_buttons)
    {

    }
    
    float x;
    float y;
    unsigned int buttons;
};

class Application
{
    public:
        Application();
        virtual ~Application();
        
        virtual double pollEvents() = 0;
        virtual void paint() = 0;
        virtual int getScreenWidth() const = 0;
        virtual int getScreenHeight() const = 0;

#ifdef ENABLE_OPENVR
        virtual int getScreenWidthVR() const = 0;
        virtual int getScreenHeightVR() const = 0;
        virtual vr::IVRSystem *getHMDVR() const = 0;
#endif

        virtual MouseState getMouseState(const bool &) = 0;
        
        void stopRunning();
        bool isRunning() const;
        bool isKeyPressed(const int &) const;
        bool isKeyPressedOnce(const int &);
        std::string getTextInput();
        
        virtual void keyDownCallback(const int &);
        virtual void keyUpCallback(const int &);
        
        void updateSimpleCamera(const float &, vec3 &, vec4 &) const;
        
    protected:
        bool pressedKeys[256];
        std::string collectedText;
        
    private:
        bool running;
};

}

}

