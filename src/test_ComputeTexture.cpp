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

#include <tiny/draw/computetexture.h>

tiny::os::Application *application = 0;
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
"   colour = vec4(texture(source, tex).xyz, 1.0f);\n"
"}\n";
    
    inputTextures.push_back("source");
    outputTextures.push_back("colour");
    
    computeTexture = new tiny::draw::ComputeTexture(inputTextures, outputTextures, fragmentShader);
    testTexture = new tiny::draw::RGBATexture2D(tiny::img::Image::createTestImage(256));
    computeTexture->setInput(*testTexture, "source");
}

void cleanup()
{
    delete computeTexture;
    delete testTexture;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    computeTexture->compute();
}

int main(int, char **)
{
    try
    {
        application = new tiny::os::SDLApplication(SCREEN_WIDTH, SCREEN_HEIGHT);
        setup();
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

