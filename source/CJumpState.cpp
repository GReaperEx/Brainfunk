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

#include "CJumpState.h"

using namespace std;

CJumpState::CJumpState(int size, ActionOnEOF onEOF, const string& dataFile)
: CVanillaState(size, 10000, false, true, onEOF, dataFile)
{}

CJumpState::~CJumpState()
{}

void CJumpState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (input.get(c)) {
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
        case '&':
        case '%':
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
        throw runtime_error("Opening and closing braces don't match.");
    }

    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(i, initData[i]);
    }
}

void CJumpState::compile(ostream&)
{
    throw runtime_error("Jumpfuck can't be compiled.");
}

void CJumpState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
    case '&':
        jumpPoints[getCell(curPtrPos)] = IP+1;
        setCell(curPtrPos, CellType{0});
    break;
    case '%':
        IP = jumpPoints[getCell(curPtrPos)] - 1;
        setCell(curPtrPos, getCell(curPtrPos+1));
    break;
    }
}
