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

#include "CSelfmodState.h"

using namespace std;

CSelfmodState::CSelfmodState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile)
: IBasicState(size, count, wrapPtr, dynamicTape, onEOF, dataFile), curPtrPos(0), IP(0)
{}

CSelfmodState::~CSelfmodState()
{}

void CSelfmodState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    while (input.get(c)) {
        switch (c)
        {
        case '[':
            ++bracesCount;
        break;
        case ']':
            --bracesCount;
        break;
        }
        setCell(curPtrPos++, CellType{(uint8_t)c});
    }
    if (bracesCount != 0) {
        throw runtime_error("Opening and closing braces don't match.");
    }

    //! Ignore the last newline
    if (getCell(curPtrPos-1).c8 == '\n') {
        setCell(--curPtrPos, CellType{0});
    }

    //! Appends collected data to tape
    for (CellType cell : initData) {
        setCell(curPtrPos++, cell);
    }
}

void CSelfmodState::run()
{
    bool keepRunning = true;

    IP = 0;
    while (keepRunning) {
        CellType curInstr = getCell(IP);
        switch (curInstr.c8)
        {
        case '>':
            ++curPtrPos;
        break;
        case '<':
            --curPtrPos;
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
            if (getCell(curPtrPos).c64 == 0) {
                int depth = 1;
                //! Make sure the brace it jumps to is the correct one, at the same level
                while (depth > 0) {
                    ++IP;
                    char token = getCell(IP).c8;
                    if (token == '[') {
                        ++depth;
                    } else if (token == ']') {
                        --depth;
                    }
                }
            }
        break;
        case ']':
            if (getCell(curPtrPos).c64 != 0) {
                int depth = 1;
                //! Make sure the brace it jumps to is the correct one, at the same level
                while (depth > 0) {
                    --IP;
                    char token = getCell(IP).c8;
                    if (token == '[') {
                        --depth;
                    } else if (token == ']') {
                        ++depth;
                    }
                }
            }
        break;
        case 0:
            keepRunning = false;
        break;
        }

        ++IP;
    }
}

void CSelfmodState::compile(ostream&)
{
    throw runtime_error("Self-modifying Brainfuck can't be compiled.");
}
