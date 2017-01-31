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

#include "CBitchanState.h"

#include <cmath>

CBitchanState::CBitchanState(int count, bool wrapPtr, bool dynamicTape, const std::string& dataFile)
: CVanillaState(1, count, wrapPtr, dynamicTape, RETM1, dataFile)
{
    if (count < 16) {
        throw std::runtime_error("Bitchanger tape must be at least 16 cells long.");
    }

    curPtrPos = 16;

    //! Doing this to avoid the parent class complaining about cell count
    cellCount = count/8;
    tape = realloc(tape, cellCount);
    memset(tape, 0, cellCount);
}

CBitchanState::~CBitchanState()
{}

void CBitchanState::translate(std::istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (input.get(c)) {
        switch (c)
        {
        case '<':
            //! A way to "optimize"/compress BF code, add many consecutive commands together
            if (!instructions.empty() && instructions.back().token == c) {
                instructions.back().incr();
            } else {
                instructions.push_back(BFinstr(c));
            }
        break;
        case '}':
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

const IBasicState::CellType CBitchanState::getCell(int cellIndex)
{
    int sign = (cellIndex >> (sizeof(int)*8 - 1)) & 0x1;

    int absIndex = (cellIndex - sign*7)/8;
    int relIndex = (cellIndex % 8 + 8) % 8;

    CellType toReturn{0};

    toReturn.c8 = (CVanillaState::getCell(absIndex).c8 >> relIndex) & 0x1;

    return toReturn;
}

void CBitchanState::setCell(int cellIndex, const CellType& newValue)
{
    int sign = (cellIndex >> (sizeof(int)*8 - 1)) & 0x1;

    int absIndex = (cellIndex - sign*7)/8;
    int relIndex = (cellIndex % 8 + 8) % 8;

    CellType temp = CVanillaState::getCell(absIndex);
    temp.c8 &= ~(0x1 << relIndex);
    temp.c8 |= (newValue.c8 & 0x1) << relIndex;

    CVanillaState::setCell(absIndex, temp);
}

void CBitchanState::compilePreInst(std::ostream& output)
{
    using std::endl;
    using std::max;

    output << "CellType* p = calloc(" << max(cellCount, (int)initData.size()+2) << ", sizeof(CellType));" << endl;
    output << "int index = 16;" << endl;
    output << "int size = " << max(cellCount, (int)initData.size()+2) << ';' << endl;

    if (!initData.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(datArray)/sizeof(CellType); i++) {" << endl;
        output << "p[i+2] = datArray[i];" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }
}

void CBitchanState::runInstruction(const BFinstr& instr)
{
    switch (instr.token)
    {
    case '<':
        curPtrPos -= instr.repeat;
    break;
    case '}':
    {
        CellType temp = getCell(curPtrPos);
        ++temp.c64;
        setCell(curPtrPos, temp);

        ++curPtrPos;
    }
    break;
    case '[':
        if (getCell(curPtrPos).c64 == 0) {
            int depth = 1;
            //! Make sure the brace it jumps to is the correct one, at the same level
            while (depth > 0) {
                ++IP;
                char token = getCode(IP).token;
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
                char token = getCode(IP).token;
                if (token == '[') {
                    --depth;
                } else if (token == ']') {
                    ++depth;
                }
            }
        }
    break;
    }

    if (getCell(5).c8) {
        if (getCell(7).c8) {
            std::cout.put((char)((uint8_t*)tape)[1]);
        } else {
            uint8_t c;
            userInput(c);

            if (c == 0xFF) {
                setCell(6, CellType{1});
            } else {
                setCell(6, CellType{0});
            }

            ((uint8_t*)tape)[1] = c;
        }
        setCell(5, CellType{0});
    }

    if (IP + 1 >= instructions.size()) {
        keepRunning = false;
    }
}

void CBitchanState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    using std::endl;

    switch (instr.token)
    {
    case '<':
        output << "index -= " << instr.repeat << ';' << endl;
        output << "if (index < 0) {" << endl;
        if (ptrWrap) {
            output << "index = " << cellCount*8 << " + index % " << cellCount*8 << ';' << endl;
        } else {
            output << "decError();" << endl;
        }
        output << "}" << endl;
    break;
    case '}':
    {
        output << "p[index/8] ^= 0x1 << (index % 8);" << endl;
        if (ptrWrap) {
            output << "index = (index + 1) % " << cellCount*8 << ';' << endl;
        } else {
            output << "index += 1;" << endl;
            output << "if (index/8 >= size) {" << endl;
            if (dynamic) {
                output << "p = incReallocPtr(p, &size, index/8);" << endl;
            } else {
                output << "incError();" << endl;
            }
            output << "}" << endl;
        }

        output << "if (p[0] & 0x20) {" << endl;
        output << "if (p[0] & 0x80) {" << endl;
        output << "putchar(p[1]);" << endl;
        output << "} else {" << endl;
        output << "int temp = getchar();" << endl;
        output << "if (temp == EOF) {" << endl;
        output << "p[0] |= 0x40;" << endl;
        output << "} else {" << endl;
        output << "p[0] &= ~0x40;" << endl;
        output << "}" << endl;
        output << "p[1] = temp;" << endl;
        output << "}" << endl;
        output << "p[0] &= ~0x20;" << endl;
        output << "}" << endl;
    }
    break;
    case '[':
        output << "while (p[index/8] & (0x1 << (index % 8))) {" << endl;
    break;
    case ']':
        output << "}" << endl;
    break;
    }
}
