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

#include "CDrawState.h"

#include <limits>

#include <signal.h>

CDrawState::CDrawState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug),
  R(0), G(0), B(0), X(0), Y(0), outimg(256, 256)
{
    //! png++ doesn't seem to have a method for filling the image with a single color
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            outimg.set_pixel(j, i, png::basic_rgb_pixel<uint8_t>(255, 255, 255));
        }
    }
}

CDrawState::~CDrawState()
{
    outimg.write("output.png");
}

void CDrawState::translate(std::istream& input)
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
        case 'r':
        case 'g':
        case 'b':
        case 'x':
        case 'y':
        case 'n':
        case 's':
        case 'e':
        case 'w':
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

void CDrawState::compile(std::ostream&)
{
    throw std::runtime_error("Cannot compile Drawfuck code.");
}

void CDrawState::runInstruction(const BFinstr& instr)
{
    switch (instr.token)
    {
    case '>':
        curPtrPos += instr.repeat;
    break;
    case '<':
        curPtrPos -= instr.repeat;
    break;
    case '+':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 += instr.repeat;
        setCell(curPtrPos, temp);
    }
    break;
    case '-':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 -= instr.repeat;
        setCell(curPtrPos, temp);
    }
    break;
    case '.':
        outimg.set_pixel(X, Y, png::basic_rgb_pixel<uint8_t>(R, G, B));
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
    case 'r':
        R = getCell(curPtrPos).c8;
    break;
    case 'g':
        G = getCell(curPtrPos).c8;
    break;
    case 'b':
        B = getCell(curPtrPos).c8;
    break;
    case 'x':
        X = getCell(curPtrPos).c8;
    break;
    case 'y':
        Y = getCell(curPtrPos).c8;
    break;
    case 'n':
        Y -= instr.repeat;
    break;
    case 's':
        Y += instr.repeat;
    break;
    case 'e':
        X += instr.repeat;
    break;
    case 'w':
        X -= instr.repeat;
    break;
    }

    if (IP + 1 >= instructions.size()) {
        keepRunning = false;
    }
}

void CDrawState::runDebug()
{
    using std::cout;
    using std::endl;
    using std::cin;

    if (dbgPaused) {
        cin.clear();
        for (;;) {
            BFinstr tempCode = getCode(IP);
            cout << "-----------------" << endl;
            cout << "Current IP      : " << IP << endl;
            cout << "Current Pointer : " << curPtrPos << endl;
            cout << "Current Color   : (" << (int)R << ", " << (int)G << ", " << (int)B << ")" << endl;
            cout << "Current Pixel   : (" << (int)X << ", " << (int)Y << ")" << endl;
            cout << "Next instruction: " << tempCode.token;
            if (tempCode.repeat > 1) {
                cout << " x" << tempCode.repeat;
            }
            cout << endl;

            char choice;
            cout << "What to do ( h ): ";
            cin >> choice;
            switch (choice)
            {
            case 'h':
                cout << "h     ; Prints this helpful list" << endl;
                cout << "a     ; Aborts the interpreter" << endl;
                cout << "n     ; Runs the next instruction" << endl;
                cout << "r     ; Resumes normal execution" << endl;
                cout << "g N   ; Prints the value of the Nth tape cell( zero-based )" << endl;
                cout << "s N X ; Gives a new value to the Nth tape cell( zero-based )" << endl;
                cout << "f     ; Flush current image to the file" << endl;
            break;
            case 'a':
                exit(-1);
            break;
            case 'n':
                return;
            break;
            case 'r':
                dbgPaused = false;
                return;
            break;
            case 'g':
            {
                int index;
                while (!(cin >> index)) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                cout << "Cell at #" << index << ':' << endl;
                cout << "        " << getCell(index).c64 << endl;
            }
            break;
            case 's':
            {
                int index;
                while (!(cin >> index)) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }
                CellType newVal;
                while (!(cin >> newVal.c64)) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }

                setCell(index, newVal);
            }
            break;
            case 'f':
                outimg.write("output.png");
            break;
            }
        }
    }
}

