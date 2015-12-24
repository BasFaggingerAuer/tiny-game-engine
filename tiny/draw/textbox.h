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
            float size; /**< The (vertical) font size (note that the screen bottom is at -1 and the top is at +1). */
            float aspectRatio; /**< The aspect ratio used for the font. */
            vec4 box; /**< The width of the text box. */
            IconTexture2D * iconMap; /**< The font map. */

            /** The list of all text fragments of the Text box. New lines are
              * started by inserting a text fragment with an empty text string. */
            std::vector<TextFragment> textFragments;

            size_t length(void) const;
        public:
            TextBox(IconTexture2D * _iconMap, const float &_size = 0.2f, const float &_aspectRatio = 1.0f);

            ~TextBox(void);

            /** Add a text fragment (a text string plus it colour) to be appended to currently
              * existing text for the TextBox. Note that text fragments are not rendered until
              * after TextBox::setText() is called. */
            void addTextFragment(std::string _text, Colour _colour);

            /** Add a newline after the currently pushed text fragments. */
            void addNewline(void);

            /** Reset the font texture. Since different fonts have different widths, we also
              * need to re-set the text. */
            void setFontTexture(IconTexture2D * _iconMap);

            /** Set the maximal dimensions of the box, forbidding rendering of text outside of
              * the square with upper-left corner (_x,_y) and lower-right corner (_p,_q), using
              * screen coordinates where (-1,1) is the upper right of the screen and (1,-1) is
              * the lower right. */
            void setBoxDimensions(const float &_x, const float &_y, const float &_p, const float &_q);

            /** Reserve enough space such that the text can be rendered fully. */
            Renderable * reserve(Renderable * &currentRenderable);

            /** Clear the contents (i.e. all text fragments) of the TextBox. */
            void clear(void);

            /** Set this Text to appear to the top-right of the location
              * specified by screen coordinates (_x,_y), where both coordinates
              * can vary from -1 to +1. */
            void setText(void);

            Renderable * getRenderable(void);
    };
} // end namespace draw

} // end namespace tiny
