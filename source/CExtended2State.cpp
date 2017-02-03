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

CExtended2State::CExtended2State(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, 10000, false, true, onEOF, dataFile, debug), storagePos(0)
{
    IP = 1;
}

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

    //! Uses any data left to initialize the tape
    parseData(input);
    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(curPtrPos + i, initData[i]);
    }
}

void CExtended2State::compile(std::ostream&)
{
    throw runtime_error("Extended Brainfuck Type II can't be compiled.");
}

void CExtended2State::runInstruction(const BFinstr& instr)
{
    keepRunning = true;

    switch (instr.token)
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
        CellType temp = getCell(curPtrPos);
        userInput(temp.c8);
        setCell(curPtrPos, temp);
    }
    break;
    case '[':
    {
        CellType selCell = getCell(curPtrPos);
        if (selCell.c64 == 0) {
            int level = 1;
            int newIP = IP+1;
            while (level != 0) {
                CellType temp = getCell(newIP++);
                if (temp.c8 == ']') {
                    --level;
                } else if (temp.c8 == '[') {
                    ++level;
                } else if (temp.c8 == '@') {
                    break;
                }
            }
            if (level == 0) {
                IP = newIP - 1;
            } else {
                CellType temp = getCell(IP);
                temp.c8 = 1;
                setCell(IP, temp);
            }
        }
    }
    break;
    case ']':
    {
        CellType selCell = getCell(curPtrPos);
        if (selCell.c64 != 0) {
            int level = 1;
            int newIP = IP-1;
            while (level != 0 && newIP >= 0) {
                CellType temp = getCell(newIP--);
                if (temp.c8 == '[') {
                    --level;
                } else if (temp.c8 == ']') {
                    ++level;
                }
            }
            if (level == 0) {
                IP = newIP+1;
            } else {
                CellType temp = getCell(IP);
                temp.c8 = 1;
                setCell(IP, temp);
            }
        }
    }
    break;
    case '@':
        keepRunning = false;
    break;
    case '$':
        setCell(storagePos, getCell(curPtrPos));
    break;
    case '!':
        setCell(curPtrPos, getCell(storagePos));
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
        temp.c64 ^= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '&':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 &= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '|':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 |= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '?':
        IP = curPtrPos - 1;
    break;
    case '(':
        for (int i = curPtrPos; i < cellCount-1; i++) {
            setCell(i, getCell(i+1));
        }
    break;
    case ')':
    {
        //! Avoids lengthening the tape if it doesn't have to
        CellType temp = getCell(cellCount-1);
        int i = cellCount;
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
        temp.c64 *= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '/':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 /= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '=':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 += getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '_':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 -= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '%':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 %= getCell(storagePos).c64;
        setCell(curPtrPos, temp);
    }
    break;
    }
}

CVanillaState::BFinstr& CExtended2State::getCode(int ip)
{
    static BFinstr localTemp(0);
    localTemp.token = getCell(ip).c8;
    localTemp.repeat = 1;

    return localTemp;
}
