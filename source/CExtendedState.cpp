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

 #include "CExtendedState.h"

 #include <limits>

using namespace std;

CExtendedState::CExtendedState(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, 10000, false, true, onEOF, dataFile, debug), storage({0})
{}

CExtendedState::~CExtendedState()
{}

void CExtendedState::translate(istream& input)
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
        case '}':
        case '{':
            //! A way to "optimize"/compress BF code, add many consecutive commands together
            if (!instructions.empty() && instructions.back().token == c) {
                instructions.back().incr();
            } else {
                instructions.push_back(BFinstr(c));
            }
        break;
        case '.':
        case ',':
        case '@':
        case '$':
        case '!':
        case '~':
        case '^':
        case '&':
        case '|':
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

void CExtendedState::compilePreInst(std::ostream& output)
{
    output << "CellType storage = 0;" << endl;
    CVanillaState::compilePreInst(output);
}

void CExtendedState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
    case '@':
        keepRunning = false;
    break;
    case '$':
        storage = getCell(curPtrPos);
    break;
    case '!':
        setCell(curPtrPos, storage);
    break;
    case '{':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 <<= instr.repeat;
        setCell(curPtrPos, temp);
    }
    break;
    case '}':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 >>= instr.repeat;
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
        temp.c64 ^= storage.c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '&':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 &= storage.c64;
        setCell(curPtrPos, temp);
    }
    break;
    case '|':
    {
        CellType temp = getCell(curPtrPos);
        temp.c64 |= storage.c64;
        setCell(curPtrPos, temp);
    }
    break;
    }
}

void CExtendedState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    int repeat = instr.repeat;
    switch (instr.token)
    {
    case '@':
        output << "exit(0);" << endl;
    break;
    case '$':
        output << "storage = p[index];" << endl;
    break;
    case '!':
        output << "p[index] = storage;" << endl;
    break;
    case '{':
        output << "p[index] <<= " << repeat << ';' << endl;
    break;
    case '}':
        output << "p[index] >>= " << repeat << ';' << endl;
    break;
    case '~':
        output << "p[index] = ~p[index];" << endl;
    break;
    case '^':
        output << "p[index] ^= storage;" << endl;
    break;
    case '&':
        output << "p[index] &= storage;" << endl;
    break;
    case '|':
        output << "p[index] |= storage;" << endl;
    break;
    }
}

void CExtendedState::runDebug()
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
            cout << "Current Storage : " << getStorage().c64 << endl;
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
            }
        }
    }
}
