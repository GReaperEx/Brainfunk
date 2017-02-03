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

#include "CCompressedState.h"

CCompressedState::CCompressedState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug), bufCmd(0), bufBits(0)
{}

CCompressedState::~CCompressedState()
{}

void CCompressedState::translate(std::istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    const char cmds[] = "+-<>.,[]";
    int numCmd;

    instructions.clear();

    while (parseInput(input, numCmd)) {
        c = cmds[numCmd];
        switch (c)
        {
        case '>':
        case '<':
        case '+':
        case '-':
            //! A way to "optimize"/compress BF code, add many consecutive commands together
            if (!instructions.empty() && instructions.back().token == c) {
                instructions.back().incr();
            } else {
                instructions.push_back(BFinstr(c));
            }
        break;
        case '.':
        case ',':
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

bool CCompressedState::parseInput(std::istream& input, int& cmd)
{
    char c;

    while (bufBits + 8 <= (signed)sizeof(bufCmd)*8) {
        if (input.get(c)) {
            bufCmd <<= 8;
            bufCmd |= c & 0xFF;
            bufBits += 8;
        } else {
            break;
        }
    }

    if (bufBits == 0) {
        return false;
    } else if (bufBits < 3) {
        cmd = (bufBits << (3 - bufBits)) & 0x7;
        bufBits = 0;
    } else {
        cmd = (bufCmd >> (bufBits - 3)) & 0x7;
        bufBits -= 3;
    }

    return true;
}

