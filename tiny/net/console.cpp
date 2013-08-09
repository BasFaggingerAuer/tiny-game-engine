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
#include <sstream>

#include <tiny/net/console.h>

using namespace tiny::net;

Console::Console() :
        lines(),
        lineScroll(0)
{
    
}

Console::~Console()
{
    
}

void Console::scrollUp()
{
    lineScroll += 10;

    if (lineScroll >= (int)lines.size()) lineScroll = lines.size() - 1;
}

void Console::scrollDown()
{
    lineScroll -= 10;
    
    if (lineScroll < 0) lineScroll = 0;
}

void Console::scrollDownFull()
{
    lineScroll = 0;
}

void Console::keyDown(const int &key)
{
    if (key == '\n' || key == '\r')
    {
        addLine(curLine);
        execute(curLine);
        curLine = "";
    }
    else if (key == '\b')
    {
        if (curLine.size() > 0) curLine.erase(curLine.size() - 1, curLine.size());
    }
    else if (key == ' ')
    {
        curLine += ' ';
    }
    else if (key >= 32 && key < 255)
    {
        curLine += key;
    }
}

void Console::addLine(const std::string &message)
{
    std::cerr << "Console: " << message << std::endl;
    lines.push_back(message);
}

void Console::execute(const std::string &)
{
    
}

std::string Console::getText(const int &nrLines) const
{
    std::ostringstream stream;
    std::vector<std::string>::const_reverse_iterator ptr = lines.rbegin() + lineScroll;
    
    stream << " $ " << curLine;
    
    for (int i = 0; i < nrLines && ptr != lines.rend(); ++i)
    {
        stream << std::endl;
        stream << "\\w\\3" << *ptr++;
    }
    
    return stream.str();
}

