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

#include <tiny/math/vec.h>

namespace tiny
{

namespace os
{

class Application
{
    public:
        Application();
        virtual ~Application();
        
        virtual double pollEvents() = 0;
        virtual void paint() = 0;
        virtual int getScreenWidth() const = 0;
        virtual int getScreenHeight() const = 0;
        
        bool isRunning() const;
        bool isKeyPressed(const int &) const;
        
        void updateSimpleCamera(const float &, vec3 &, vec4 &) const;
        
    protected:
        void stopRunning();
        
        bool pressedKeys[256];
        
    private:
        bool running;
};

}

}

