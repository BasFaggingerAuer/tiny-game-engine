/*
Copyright 2015, Matthijs van Dorp.

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

#include <tiny/draw/textbox.h>

using namespace tiny;
using namespace tiny::draw;

size_t TextBox::length(void) const
{
    size_t l = 0;
    for(unsigned int i = 0; i < textFragments.size(); i++)
        l += textFragments[i].text.length();
    return l;
}


TextBox::TextBox(IconTexture2D * _iconMap, const float &_size, const float &_aspectRatio) :
    iconHorde(0), size(_size), aspectRatio(_aspectRatio), box(-1.0f,1.0f,1.0f,-1.0f), iconMap(_iconMap)
{
    iconHorde = new ScreenIconHorde( 1 );
    iconHorde->setIconTexture(*iconMap);
}

TextBox::~TextBox(void)
{
    if(iconHorde)
    {
        delete iconHorde;
        iconHorde = 0;
    }
}

void TextBox::addTextFragment(std::string _text, Colour _colour)
{
    textFragments.push_back(TextFragment(_text, _colour));
}

Renderable * TextBox::reserve(Renderable * &currentRenderable)
{
    if(!iconHorde || iconHorde->maxNumIcons() <= length())
    {
        std::cout << " setText() : Reset iconHorde "<<iconHorde<<" to length "<<length()<<"... "<<std::endl;
        if(iconHorde)
        {
            currentRenderable = iconHorde;
            delete iconHorde;
        }
        iconHorde = new ScreenIconHorde( (unsigned int)(1+length()*1.2) );
        iconHorde->setIconTexture(*iconMap);
        return iconHorde;
    }
    else return 0;
}

void TextBox::setFontTexture(IconTexture2D * _iconMap);
{
    iconMap = _iconMap;
    iconHorde->setIconTexture(*iconMap);
    setText();
}

void TextBox::setBoxDimensions(const float &_x, const float &_y, const float &_p, const float &_q)
{
    box = vec4(_x,_y,_p,_q);
}

void TextBox::setText(void)
{
    iconHorde->eraseText();
    vec4 iconPos(box.x, box.y-size, 0.0f, 0.0f);
    for(unsigned int i = 0; i < textFragments.size(); i++)
    {
        iconHorde->appendText(iconPos, size, aspectRatio, textFragments[i].text, *iconMap, textFragments[i].colour, box);
    }
}

Renderable * TextBox::getRenderable(void)
{
    return iconHorde;
}
