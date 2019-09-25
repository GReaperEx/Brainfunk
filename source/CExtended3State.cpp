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

#include "CExtended3State.h"

#include <string>

using namespace std;

CExtended3State::CExtended3State(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CExtended2State(size, onEOF, dataFile, debug), initPtrPos(0)
{}

CExtended3State::~CExtended3State()
{}

void CExtended3State::translate(std::istream& input)
{
    string tokens = "<>+-.,[]@$!{}~^&|?()*/=_%XxMmLl:#0123456789ABCDEFabcdef";
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

    initPtrPos = curPtrPos;
    //! Uses any data left to initialize the tape
    parseData(input);
    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(curPtrPos + i, initData[i]);
    }
}

void CExtended3State::compile(std::ostream&)
{
    throw runtime_error("Extended Brainfuck Type III can't be compiled.");
}

void CExtended3State::runInstruction(const BFinstr& instr)
{
    CExtended2State::runInstruction(instr);

    CellType curCell = getCell(IP);

    switch (instr.token)
    {
    case 'X':
        prevPtrs.push_back(curPtrPos);
        curPtrPos = IP-1;
    break;
    case 'x':
        if (prevPtrs.empty()) {
            curPtrPos = initPtrPos;
        } else {
            curPtrPos = prevPtrs.back();
            prevPtrs.pop_back();
        }
    break;
    case 'M':
        storagePos = curPtrPos;
    break;
    case 'm':
        storagePos = 0;
    break;
    case 'L':
        lockMap[curPtrPos] = true;
    break;
    case 'l':
        lockMap[curPtrPos] = false;
    break;
    case ':':
        curPtrPos += (int)getCell(curPtrPos).c32; //! Using c64 is meaningless, curPtrPos is 32-bit
    break;
    case '#':
        for (;;) {
            CellType temp = getCell(++IP);
            if (temp.c8 == '#') {
                break;
            } else if (temp.c8 == '@') {
                --IP;
                break;
            }
        }
    break;
    default:
        curCell.c8 = toupper(curCell.c8);
        if (curCell.c8 >= '0' && curCell.c8 <= '9') {
            CellType temp = getCell(curPtrPos);

            temp.c8 = (curCell.c8 - '0') << 4;
            if (!isLocked(curPtrPos)) {
                setCell(curPtrPos, temp);
            }
        } else if (curCell.c8 >= 'A' && curCell.c8 <= 'F') {
            CellType temp = getCell(curPtrPos);

            temp.c8 = (curCell.c8 - 'A' + 10) << 4;
            if (!isLocked(curPtrPos)) {
                setCell(curPtrPos, temp);
            }
        }
    }
}
