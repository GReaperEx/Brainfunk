// Copyright (C) 2017, GReaperEx(Marios F.)
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CExtended2State.h"

#include <string>

using namespace std;

CExtended2State::CExtended2State(int size, const string& dataFile)
: IBasicState(size, 10000, false, true, dataFile), curPtrPos(0), IP(1)
{}

CExtended2State::~CExtended2State()
{}

void CExtended2State::translate(std::istream& input)
{
    string tokens = "<>+-.,[]@$!{}~^&|?()*/=_%";
    char c;
    bool endFound = false;

    ++curPtrPos;
    while (input >> c) {
        if (tokens.find(c) != string::npos) {
            CellType temp = { 0 };
            temp.c8 = c;
            setCell(curPtrPos++, temp);

            if (c == '@') {
                endFound = true;
                break;
            }
        }
    }

    //! Adds the '@' instruction if none was given
    if (!endFound) {
        CellType temp = { 0 };
        temp.c8 = '@';
        setCell(curPtrPos++, temp);
    }

    int initPtrPos = curPtrPos;
    //! Uses any data left to initialize the tape
    curPtrPos = parseData(curPtrPos, input);
    //! Appends data file to tape
    stringstream data(initData);
    parseData(curPtrPos, data);
    curPtrPos = initPtrPos;
}

void CExtended2State::run()
{
    CellType curCell = { 0 };

    while (curCell.c8 != '@') {
        curCell = getCell(IP++);
        switch (curCell.c8)
        {
        case '<':
            --curPtrPos;
        break;
        case '>':
            ++curPtrPos;
        break;
        case '+':
        {
            CellType temp = getCell(curPtrPos);
            ++temp.c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '-':
        {
            CellType temp = getCell(curPtrPos);
            --temp.c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '.':
        {
            CellType temp = getCell(curPtrPos);
            cout.put(temp.c8);
        }
        break;
        case ',':
        {
            CellType temp = { 0 };
            temp.c8 = cin.get();
            setCell(curPtrPos, temp);
        }
        break;
        case '[':
        {
            CellType selCell = getCell(curPtrPos);
            if (selCell.c64 == 0) {
                int level = 1;
                int newIP = IP;
                while (level != 0) {
                    CellType temp = getCell(newIP++);
                    if (temp.c8 == ']') {
                        --level;
                    } else if (temp.c8 == '@') {
                        break;
                    }
                }
                if (level == 0) {
                    IP = newIP;
                } else {
                    CellType temp = getCell(IP-1);
                    temp.c8 = 1;
                    setCell(IP-1, temp);
                }
            }
        }
        break;
        case ']':
        {
            CellType selCell = getCell(curPtrPos);
            if (selCell.c64 != 0) {
                int level = 1;
                int newIP = IP-2;
                while (level != 0 && newIP >= 0) {
                    CellType temp = getCell(newIP--);
                    if (temp.c8 == '[') {
                        --level;
                    }
                }
                if (level == 0) {
                    IP = newIP+2;
                } else {
                    CellType temp = getCell(IP-1);
                    temp.c8 = 1;
                    setCell(IP-1, temp);
                }
            }
        }
        break;
        // case '@':
        // break;
        case '$':
            setCell(0, getCell(curPtrPos));
        break;
        case '!':
            setCell(curPtrPos, getCell(0));
        break;
        case '{':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 <<= 1;
            setCell(curPtrPos, temp);
        }
        break;
        case '}':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 >>= 1;
            setCell(curPtrPos, temp);
        }
        break;
        case '~':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 = ~temp.c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '^':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 ^= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '&':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 &= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '|':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 |= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '?':
            IP = curPtrPos;
        break;
        case '(':
            for (int i = curPtrPos; i < getCellCount()-1; i++) {
                setCell(i, getCell(i+1));
            }
        break;
        case ')':
        {
            //! Avoids lengthening the tape if it doesn't have to
            CellType temp = getCell(getCellCount()-1);
            int i = getCellCount();
            if (temp.c64 == 0) {
                --i;
            }

            for (; i > curPtrPos; i--) {
                setCell(i, getCell(i-1));
            }
            temp.c64 = 0;
            setCell(i, temp);
        }
        break;
        case '*':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 *= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '/':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 /= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '=':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 += getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '_':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 -= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '%':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 %= getCell(0).c64;
            setCell(curPtrPos, temp);
        }
        break;
        }
    }
}

void CExtended2State::compile(std::ostream&)
{
    throw runtime_error("Extended Brainfuck Type II can't be compiled.");
}

