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
#pragma once

#include <tiny/math/vec.h>
#include <tiny/draw/renderable.h>
#include <tiny/draw/vertexbuffer.h>
#include <tiny/draw/vertexbufferinterpreter.h>
#include <tiny/draw/icontexture2d.h>
#include <tiny/draw/iconhorde.h>
#include <tiny/draw/colour.h>

namespace tiny
{

namespace draw
{
    /** The TextBox displays text in the form of a collection of
      * screen-coordinate icons. It employs the ScreenIconHorde
      * for this. It is better suited for drawing variable texts
      * than the ScreenIconHorde itself, which contains only the
      * simplest possible text rendering functionality. */
    class TextBox
    {
        private:
            struct TextFragment
            {
                std::string text;
                Colour colour;
                TextFragment(std::string s, Colour c) : text(s), colour(c) {}
            };

            ScreenIconHorde * iconHorde;
            float size; /**< The font size. */
            float aspectRatio; /**< The aspect ratio used for the font. */
            vec4 box; /**< The width of the text box. */
            IconTexture2D * iconMap; /**< The font map. */

            std::vector<TextFragment> textFragments;

            size_t length(void) const
            {
                size_t l = 0;
                for(unsigned int i = 0; i < textFragments.size(); i++)
                    l += textFragments[i].text.length();
                return l;
            }
        public:
            TextBox(IconTexture2D * _iconMap, const float &_size = 0.2f, const float &_aspectRatio = 1.0f) :
                iconHorde(0), size(_size), aspectRatio(_aspectRatio), box(-1.0f,1.0f,1.0f,-1.0f), iconMap(_iconMap)
            {
                iconHorde = new ScreenIconHorde( 1 );
                iconHorde->setIconTexture(*iconMap);
            }

            ~TextBox(void)
            {
                if(iconHorde) { delete iconHorde; iconHorde = 0; }
            }

            void addTextFragment(std::string _text, Colour _colour)
            {
                textFragments.push_back(TextFragment(_text, _colour));
            }

            /** Reset the font texture. Since different fonts have different widths, we also
              * need to re-set the text. */
            void setFontTexture(IconTexture2D * _iconMap) { iconMap = _iconMap; iconHorde->setIconTexture(*iconMap); setText(); }

            /** Set the maximal dimensions of the box, forbidding rendering of text outside of
              * the square with upper-left corner (_x,_y) and lower-right corner (_p,_q), using
              * screen coordinates where (-1,1) is the upper right of the screen and (1,-1) is
              * the lower right. */
            void setBoxDimensions(const float &_x, const float &_y, const float &_p, const float &_q)
            {
                box = vec4(_x,_y,_p,_q);
            }

            /** Reserve enough space such that the text can be rendered fully. */
            Renderable * reserve(Renderable * &currentRenderable)
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

            /** Set this Text to appear to the top-right of the location
              * specified by screen coordinates (_x,_y), where both coordinates
              * can vary from -1 to +1. */
            void setText(void)
            {
                vec4 iconPos(box.x, box.y-size, 0.0f, 0.0f);
                for(unsigned int i = 0; i < textFragments.size(); i++)
                {
                    iconHorde->appendText(iconPos, size, aspectRatio, textFragments[i].text, *iconMap, textFragments[i].colour, box);
                }
//                iconHorde->setText(-1.0f,-1.0f,0.2,aspectRatio,"Test",*iconMap);
            }

            Renderable * getRenderable(void)
            {
                return iconHorde;
            }
    };
} // end namespace draw

} // end namespace tiny
