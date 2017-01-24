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

#ifndef IBASIC_STATE_H
#define IBASIC_STATE_H

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <sstream>
#include <iomanip>

class IBasicState
{
public:
    /**
        size        : The size of each cell, acceptable values are 1, 2, 4 or 8
        count       : The amount of cells available, meaningless if dynamicTape == true
        wrapPtr     : Wraps the tape pointer around, can't be true if dynamicTape is also true
        dynamicTape : Makes the available tape grow dynamically when accessing out of upper bounds
     */
    IBasicState(int size, int count, bool wrapPtr, bool dynamicTape, const std::string& dataFile) {
        if (size != 1 && size != 2 && size != 4 && size != 8) {
            throw std::runtime_error("Invalid cell size. Only 1, 2, 4 and 8 are supported.");
        }
        if (count <= 0 && !dynamicTape) {
            throw std::runtime_error("Invalid cell count. Must be greater than zero.");
        } else if (count < 9999 && !dynamicTape) {
            std::cerr << "Warning: Having less than 9999 cells isn't considered \"nice\"." << std::endl;
        }
        if (wrapPtr && wrapPtr == dynamicTape) {
            throw std::runtime_error("Cannot have both a dynamic tape and a wrap-around pointer.");
        }

        cellSize = size;

        if (dynamicTape) {
            cellCount = 10000;
        } else {
            cellCount = count;
        }

        ptrWrap = wrapPtr;
        dynamic = dynamicTape;

        tape = calloc(cellCount, cellSize);
        if (tape == nullptr) {
            throw std::runtime_error("There's not enough memory available!");
        }

        if (!dataFile.empty()) {
            std::ifstream input(dataFile);
            if (!input.is_open()) {
                throw std::runtime_error("Unable to open requested data file.");
            }

            input.seekg(0, std::ios::end);
            initData.reserve(input.tellg());
            input.seekg(0, std::ios::beg);

            initData.assign((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        }
    }

    virtual ~IBasicState() {
        free(tape);
    }

    virtual void translate(std::istream& input) = 0;
    virtual void run() = 0;
    virtual void compile(std::ostream& output) = 0;

protected:
    union CellType
    {
        uint8_t  c8;
        uint16_t c16;
        uint32_t c32;
        uint64_t c64;
    };

    int getCellSize() const {
        return cellSize;
    }

    int getCellCount() const {
        return cellCount;
    }

    bool wrapsPointer() const {
        return ptrWrap;
    }

    bool isDynamic() const {
        return dynamic;
    }

    const CellType getCell(int cellIndex) {
        CellType toReturn = { 0 };

        examineIndex(cellIndex);

        switch (cellSize)
        {
        case 1:
            toReturn.c8 = ((uint8_t*)tape)[cellIndex];
        break;
        case 2:
            toReturn.c16 = ((uint16_t*)tape)[cellIndex];
        break;
        case 4:
            toReturn.c32 = ((uint32_t*)tape)[cellIndex];
        break;
        case 8:
            toReturn.c64 = ((uint64_t*)tape)[cellIndex];
        break;
        }

        return toReturn;
    }

    void setCell(int cellIndex, const CellType& newValue) {
        examineIndex(cellIndex);

        switch (cellSize)
        {
        case 1:
            ((uint8_t*)tape)[cellIndex] = newValue.c8;
        break;
        case 2:
            ((uint16_t*)tape)[cellIndex] = newValue.c16;
        break;
        case 4:
            ((uint32_t*)tape)[cellIndex] = newValue.c32;
        break;
        case 8:
            ((uint64_t*)tape)[cellIndex] = newValue.c64;
        break;
        }
    }

    //! Understands escape sequences, symbol, octal and hex
    //! Only hex values can be > 255( technically, octal too but not for much )
    int parseData(int startPos, std::istream& input) {
        int curPos = startPos;
        char c;

        while (input.get(c)) {
            CellType temp = { 0 };
            if (c == '\\') {
                if (!input.get(c)) {
                    throw std::runtime_error("Data: Expected symbol after \'\\\'.");
                }
                switch (c)
                {
                case 'a':
                    temp.c8 = '\a';
                break;
                case 'b':
                    temp.c8 = '\b';
                break;
                case 'f':
                    temp.c8 = '\f';
                break;
                case 'n':
                    temp.c8 = '\n';
                break;
                case 'r':
                    temp.c8 = '\r';
                break;
                case 't':
                    temp.c8 = '\t';
                break;
                case 'v':
                    temp.c8 = '\v';
                break;
                case '\\':
                    temp.c8 = '\\';
                break;
                case '\'':
                    temp.c8 = '\'';
                break;
                case '\"':
                    temp.c8 = '\"';
                break;
                case '?':
                    temp.c8 = '?';
                break;
                case 'x':
                    for (int i = 0; i < getCellSize()*2; i++) {
                        int c = tolower(input.peek());
                        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
                            break;
                        }
                        c = input.get();

                        temp.c64 <<= 4;
                        if (c >= 'a' && c <= 'f') {
                            temp.c64 += (c - 'a') + 10;
                        } else {
                            temp.c64 += (c - '0');
                        }
                    }
                break;
                default:
                    if (!(c >= '0' && c <= '7')) {
                        throw std::runtime_error("Data: Unexpected symbol after \'\\\'.");
                    }
                    for (int i = 0; i < 3; i++) {
                        int c = tolower(input.peek());
                        if (!(c >= '0' && c <= '7')) {
                            break;
                        }
                        c = input.get();

                        temp.c64 <<= 3;
                        temp.c64 += (c - '0');
                    }
                }
            } else {
                temp.c8 = c;
            }

            setCell(curPos++, temp);
        }
        return curPos;
    }

    //! Does the opposite of parseData()
    const std::string escapeData() const {
        std::string result = "";

        for (auto it = initData.begin(); it < initData.end(); it++) {
            char c = *it;
            switch (c)
                {
                case '\a':
                    result += "\\a";
                break;
                case '\b':
                    result += "\\b";
                break;
                case '\f':
                    result += "\\f";
                break;
                case '\n':
                    result += "\\n";
                break;
                case '\r':
                    result += "\\r";
                break;
                case '\t':
                    result += "\\t";
                break;
                case '\v':
                    result += "\\v";
                break;
                case '\\':
                    result += "\\\\";
                break;
                case '\'':
                    result += "\\\'";
                break;
                case '\"':
                    result += "\\\"";
                break;
                default:
                    if (!isprint(c)) {
                        std::stringstream temp;
                        temp << std::hex << (int)c;
                        result += "\\x" + temp.str();
                    } else {
                        result += c;
                    }
                }
        }

        return result;
    }

    std::string initData;

private:
    void examineIndex(int& cellIndex) {
        if (cellIndex < 0) {
            if (ptrWrap) {
                cellIndex = cellCount - cellIndex % cellCount;
            } else {
                throw std::runtime_error("Pointer was decremented too much.");
            }
        }

        if (dynamic) {
            //! Either extend the tape enough or die trying
            while (cellIndex >= cellCount) {
                int oldCount = cellCount;
                cellCount += cellCount/2 + 1;

                tape = realloc(tape, cellCount*cellSize);
                if (tape == nullptr) {
                    throw std::runtime_error("There's not enough memory available!");
                }

                memset((char*)tape + oldCount*cellSize, 0, cellCount*cellSize - oldCount*cellSize);
            }
        } else {
            if (cellIndex >= cellCount) {
                if (ptrWrap) {
                    cellIndex %= cellCount;
                } else {
                    throw std::runtime_error("Pointer was incremented too much");
                }
            }
        }
    }

    void* tape;

    int cellSize;
    int cellCount;

    bool ptrWrap;
    bool dynamic;
};

#endif // IBASIC_STATE_H
