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

#include <AL/al.h>
#include <AL/alc.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include <tiny/os/application.h>

namespace tiny
{

namespace os
{

class SDLApplication : public Application
{
    public:
        SDLApplication(const int &, const int &, const int & = 0, const int & = 24, const int & = 44100, const int & = MIX_DEFAULT_FORMAT, const int & = 2, const int & = 16, const int & = 4096);
        ~SDLApplication();
        
        double pollEvents();
        void paint();
        int getScreenWidth() const;
        int getScreenHeight() const;
        MouseState getMouseState(const bool &);
        
        virtual void keyDownCallback(const int &);
        virtual void keyUpCallback(const int &);
        
    private:
        void initOpenGL();
        void initOpenAL();
        void exitOpenAL();
        
        int screenWidth;
        int screenHeight;
        int screenBPP;
        int screenDepthBPP;
        Uint32 screenFlags;
        SDL_Surface *screen;
        Uint32 lastCount, curCount;
        bool wireframe;
        
        ALCdevice *alDevice;
        ALCcontext *alContext;
        
        int audioRate;
        int audioFormat;
        int audioChannels;
        int audioMixChannels;
        int audioBuffer;
};

}

}

