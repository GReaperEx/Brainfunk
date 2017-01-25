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

#include "CBCDState.h"

using namespace std;

CBCDState::CBCDState(int count, bool wrapPtr, bool dynamicTape, const string& dataFile)
: IBasicState(1, count, wrapPtr, dynamicTape, dataFile), curPtrPos(0), IP(0),
  swapFunctions(false), bufInput(0), isInputBuf(false), bufOutput(0), isOutputBuf(false)
{}

CBCDState::~CBCDState()
{
    if (isOutputBuf) {
        showOutput(0);
    }
}

void CBCDState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (input.get(c)) {
        switch (c & 0xF)
        {
        case '[':
            ++bracesCount;
        break;
        case ']':
            --bracesCount;
        break;
        }
        switch ((uint8_t)c >> 4)
        {
        case '[':
            ++bracesCount;
        break;
        case ']':
            --bracesCount;
        break;
        }
        instructions.push_back(c);
    }
    if (bracesCount != 0) {
        throw runtime_error("Opening and closing braces don't match.");
    }

    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(i, initData[i]);
    }
}

void CBCDState::run()
{
    IP = 0;
    while (IP < _getCodeSize()) {
        uint8_t code = _getCode(IP);
        switch (code)
        {
        case 0xA:
            --curPtrPos;
        break;
        case 0xE:
            showOutput(_getCell(curPtrPos));
        break;
        case 0xD:
            _setCell(curPtrPos, getInput());
        break;
        case 0xB:
            if (_getCell(curPtrPos) == 0) {
                int depth = 1;
                //! Make sure the brace it jumps to is the correct one, at the same level
                while (depth > 0) {
                    ++IP;
                    char token = _getCode(IP);
                    if (token == 0xB) {
                        ++depth;
                    } else if (token == 0xC) {
                        --depth;
                    }
                }
            }
        break;
        case 0xC:
            if (_getCell(curPtrPos) != 0) {
                int depth = 1;
                //! Make sure the brace it jumps to is the correct one, at the same level
                while (depth > 0) {
                    --IP;
                    char token = _getCode(IP);
                    if (token == 0xB) {
                        --depth;
                    } else if (token == 0xC) {
                        ++depth;
                    }
                }
            }
        break;
        case 0xF:
            swapFunctions = !swapFunctions;
        break;
        default:
            _setCell(curPtrPos, _getCell(curPtrPos) + code);
            ++curPtrPos;
        }

        ++IP;
    }
}

void CBCDState::compile(ostream&)
{
    throw runtime_error("BCDFuck can't be compiled.");
}

uint8_t CBCDState::_getCell(int cellIndex)
{
    unsigned realIndex = cellIndex/2;

    if (swapFunctions) {
        if (instructions.size() <= realIndex) {
            if (isDynamic()) {
                instructions.insert(instructions.end(), realIndex - instructions.size() + 1, 0);
            } else if (wrapsPointer()) {
                realIndex %= instructions.size();
            } else {
                throw std::runtime_error("Pointer was incremented too much.");
            }
        } else if (cellIndex < 0) {
            if (wrapsPointer()) {
                cellIndex = instructions.size() + cellIndex % instructions.size();
                realIndex = cellIndex/2;
            } else {
                throw std::runtime_error("Pointer was decremented too much.");
            }
        }
        return (instructions[realIndex] >> (4 - 4*(cellIndex % 2))) & 0xF;
    }
    return (getCell(realIndex).c8 >> (4 - 4*(cellIndex % 2))) & 0xF;
}

void CBCDState::_setCell(int cellIndex, uint8_t newVal)
{
    if (swapFunctions) {
        unsigned realIndex = cellIndex/2;
        if (instructions.size() <= realIndex) {
            if (isDynamic()) {
                instructions.insert(instructions.end(), realIndex - instructions.size() + 1, 0);
            } else if (wrapsPointer()) {
                realIndex %= instructions.size();
            } else {
                throw std::runtime_error("Pointer was incremented too much.");
            }
        } else if (cellIndex < 0) {
            if (wrapsPointer()) {
                cellIndex = instructions.size() + cellIndex % instructions.size();
                realIndex = cellIndex/2;
            } else {
                throw std::runtime_error("Pointer was decremented too much.");
            }
        }

        instructions[realIndex] &= 0xF0 >> (4 - 4*(cellIndex % 2));
        instructions[realIndex] |= (newVal & 0xF) << (4 - 4*(cellIndex % 2));
    } else {
        int realIndex = cellIndex/2;
        CellType temp = getCell(realIndex);
        temp.c8 &= 0xF0 >> (4 - 4*(cellIndex % 2));
        temp.c8 |= (newVal & 0xF) << (4 - 4*(cellIndex % 2));
        setCell(realIndex, temp);
    }
}

uint8_t CBCDState::_getCode(int codeIndex)
{
    swapFunctions = !swapFunctions;
    uint8_t toReturn = _getCell(codeIndex);
    swapFunctions = !swapFunctions;

    return toReturn;
}

void CBCDState::_setCode(int codeIndex, uint8_t newVal)
{
    swapFunctions = !swapFunctions;
    _setCell(codeIndex, newVal);
    swapFunctions = !swapFunctions;
}

size_t CBCDState::_getCodeSize()
{
    if (swapFunctions) {
        return getCellCount()*2;
    }
    return instructions.size()*2;
}
