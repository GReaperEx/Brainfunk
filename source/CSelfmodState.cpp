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

CSelfmodState::CSelfmodState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug)
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

void CSelfmodState::compile(ostream&)
{
    throw runtime_error("Self-modifying Brainfuck can't be compiled.");
}

void CSelfmodState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    keepRunning = true;

    if (instr.token == 0) {
        keepRunning = false;
    }
}

//! This is just a placeholder
CVanillaState::BFinstr& CSelfmodState::getCode(int ip)
{
    static BFinstr localBuffer(0);
    localBuffer.token = getCell(ip).c8;
    localBuffer.repeat = 1;

    return localBuffer;
}
