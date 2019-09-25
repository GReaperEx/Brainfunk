// Copyright (C) 2017-2019, GReaperEx(Marios F.)
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 only.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CExtLollerState.h"

#include <map>

#include <png++/png.hpp>

CExtLollerState::CExtLollerState(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CExtendedState(size, onEOF, dataFile, debug)
{}

CExtLollerState::~CExtLollerState()
{}

struct customCmp
{
    bool operator() (const png::rgb_pixel& a, const png::rgb_pixel& b) const {
        if (a.red < b.red) {
            return true;
        } else if (a.red > b.red) {
            return false;
        }
        if (a.green < b.green) {
            return true;
        } else if (a.green > b.green) {
            return false;
        }
        return a.blue < b.blue;
    }
};

void CExtLollerState::translate(std::istream& input)
{
    png::image<png::rgb_pixel> img(input);

    std::map<png::rgb_pixel, char, customCmp> colorMap;
    colorMap[png::rgb_pixel(255, 0, 0)] = '>';
    colorMap[png::rgb_pixel(128, 0, 0)] = '<';
    colorMap[png::rgb_pixel(0, 255, 0)] = '+';
    colorMap[png::rgb_pixel(0, 128, 0)] = '-';
    colorMap[png::rgb_pixel(0, 0, 255)] = '.';
    colorMap[png::rgb_pixel(0, 0, 128)] = ',';
    colorMap[png::rgb_pixel(255, 255, 0)] = '[';
    colorMap[png::rgb_pixel(128, 128, 0)] = ']';
    colorMap[png::rgb_pixel(0, 255, 255)] = 'r';
    colorMap[png::rgb_pixel(0, 128, 128)] = 'R';

    colorMap[png::rgb_pixel(0, 192, 64)] = '@';
    colorMap[png::rgb_pixel(192, 64, 0)] = '$';
    colorMap[png::rgb_pixel(64, 0, 192)] = '!';
    colorMap[png::rgb_pixel(64, 192, 0)] = '}';
    colorMap[png::rgb_pixel(192, 0, 64)] = '{';
    colorMap[png::rgb_pixel(0, 64, 192)] = '~';
    colorMap[png::rgb_pixel(0,  192, 0)] = '^';
    colorMap[png::rgb_pixel(192, 0,  0)] = '&';
    colorMap[png::rgb_pixel(0,  0, 192)] = '|';

    size_t curX = 0;
    size_t curY = 0;
    int curDir = 0;

    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (curX < img.get_width() && curY < img.get_height()) {
        png::rgb_pixel tempPixel = img.get_pixel(curX, curY);
        auto iter = colorMap.find(tempPixel);
        if (iter != colorMap.end()) {
            char c = iter->second;
            switch (c)
            {
            case '>':
            case '<':
            case '+':
            case '-':
            case '{':
            case '}':
                //! A way to "optimize"/compress BF code, add many consecutive commands together
                if (!instructions.empty() && instructions.back().token == c) {
                    instructions.back().incr();
                } else {
                    instructions.push_back(BFinstr(c));
                }
            break;
            case '.':
            case ',':
            case '@':
            case '$':
            case '!':
            case '~':
            case '^':
            case '&':
            case '|':
                instructions.push_back(BFinstr(c));
            break;
            case '[':
                ++bracesCount;
                instructions.push_back(BFinstr(c));
            break;
            case ']':
                --bracesCount;
                instructions.push_back(BFinstr(c));
            break;
            case 'R':
                curDir = (curDir + 1) % 4;
            break;
            case 'r':
                curDir = (curDir + 3) % 4;
            break;
            }
        }

        switch (curDir)
        {
        case 0:
            ++curX;
        break;
        case 1:
            --curY;
        break;
        case 2:
            --curX;
        break;
        case 3:
            ++curY;
        break;
        }
    }

    if (bracesCount != 0) {
        throw std::runtime_error("Opening and closing braces don't match.");
    }

    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(i, initData[i]);
    }
}
