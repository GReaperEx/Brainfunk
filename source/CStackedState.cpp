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

#include "CStackedState.h"

#include <limits>

using namespace std;

CStackedState::CStackedState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug)
{}

CStackedState::~CStackedState()
{}

void CStackedState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (input.get(c) && c != '!') {
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
        case '(':
        case ')':
        case '@':
        case '$':
        case '=':
        case '_':
        case '{':
        case '}':
        case '|':
        case '^':
        case '&':
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
    //! Save any remaining data on the stack
    if (c == '!') {
        while (input.get(c)) {
            CellType temp { 0 };
            temp.c8 = c;
            cellStack.push_back(temp);
        }
    }
    //! It should ignore the last newline
    if (!cellStack.empty() && cellStack.back().c8 == '\n') {
        cellStack.pop_back();
    }
}

void CStackedState::compilePreMain(std::ostream& output)
{
    CVanillaState::compilePreMain(output);

    output << "void* pushStack(CellType* p, int* size, int* index, CellType newVal) {" << endl;
    output << "++*index;" << endl;
    output << "if (*index == *size) {" << endl;
    output << "p = realloc(p, (++*size)*sizeof(CellType));" << endl;
    output << "if (!p) {" << endl;
    output << "fputs(\"Out of memory!\\n\", stderr);" << endl;
    output << "exit(-1);" << endl;
    output << "}" << endl;
    output << "}" << endl;
    output << "((CellType*)p)[*index] = newVal;" << endl;
    output << "return p;" << endl;
    output << "}" << endl;

    output << "CellType peekStack(CellType* p, int index) {" << endl;
    output << "CellType toReturn = 0;" << endl;
    output << "if (index >= 0) {" << endl;
    output << "toReturn = p[index];" << endl;
    output << "}" << endl;
    output << "return toReturn;" << endl;
    output << "}" << endl;

    output << "CellType popStack(CellType* p, int* index) {" << endl;
    output << "CellType toReturn = 0;" << endl;
    output << "if (*index >= 0) {" << endl;
    output << "toReturn = p[*index];" << endl;
    output << "--*index;" << endl;
    output << "}" << endl;
    output << "return toReturn;" << endl;
    output << "}" << endl;

    if (!cellStack.empty()) {
        output << "const CellType stackData[] = { ";
        for (CellType cell : cellStack) {
            output << "0x" << hex << cell.c64 << ',';
        }
        output << " };" << endl;
    }
}

void CStackedState::compilePreInst(std::ostream& output)
{
    output << "CellType* pS = NULL;" << endl;
    output << "int sIndex = -1;" << endl;
    output << "int sSize = 0;" << endl;
    output << "CellType temp;" << endl;

    CVanillaState::compilePreInst(output);

    if (!cellStack.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(stackData)/sizeof(stackData[0]); i++) {" << endl;
        output << "pS = pushStack(pS, &sSize, &sIndex, stackData[i]);" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }
}

void CStackedState::compileCleanup(std::ostream& output)
{
    CVanillaState::compileCleanup(output);
    output << "free(pS);" << endl;
}

void CStackedState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
    case '(':
        if (!cellStack.empty()) {
            setCell(curPtrPos, cellStack.back());
            cellStack.pop_back();
        } else {
            CellType temp = { 0 };
            setCell(curPtrPos, temp);
        }
    break;
    case ')':
        cellStack.push_back(getCell(curPtrPos));
    break;
    case '@':
        if (!cellStack.empty()) {
            setCell(curPtrPos, cellStack.back());
        } else {
            CellType temp = { 0 };
            setCell(curPtrPos, temp);
        }
    break;
    case '$':
        if (!cellStack.empty()) {
            cellStack.pop_back();
        }
    break;
    case '=':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 += temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '_':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 -= temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '{':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 <<= temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '}':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 >>= temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '|':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 |= temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '^':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 ^= temp2.c64;
            setCell(curPtrPos, temp1);
        }
    break;
    case '&':
        if (!cellStack.empty()) {
            CellType temp1 = getCell(curPtrPos);
            CellType temp2 = cellStack.back();
            temp1.c64 += temp2.c64;
            setCell(curPtrPos, temp1);
        } else {
            CellType temp = { 0 };
            setCell(curPtrPos, temp);
        }
    break;
    }
}

void CStackedState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    CVanillaState::compileInstruction(output, instr);

    switch (instr.token)
    {
    case '(':
        output << "p[index] = popStack(pS, &sIndex);" << endl;
    break;
    case ')':
        output << "pS = pushStack(pS, &sSize, &sIndex, p[index]);" << endl;
    break;
    case '@':
        output << "p[index] = peekStack(pS, sIndex);" << endl;
    break;
    case '$':
        output << "(void)popStack(pS, &sIndex);" << endl;
    break;
    case '=':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] += temp;" << endl;
    break;
    case '_':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] -= temp;" << endl;
    break;
    case '{':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] <<= temp;" << endl;
    break;
    case '}':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] >>= temp;" << endl;
    break;
    case '|':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] |= temp;" << endl;
    break;
    case '^':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] ^= temp;" << endl;
    break;
    case '&':
        output << "temp = peekStack(pS, sIndex);" << endl;
        output << "p[index] &= temp;" << endl;
    break;
    }
}

void CStackedState::runDebug()
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
            cout << "Current Pointer :" << curPtrPos << endl;
            cout << "Stack size      : " << cellStack.size() << endl;
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
                cout << "G     ; Peeks top stack value" << endl;
                cout << "S X   ; Replaces top stack value" << endl;
                cout << "P X   ; Pushes value unto the stack" << endl;
                cout << "p     ; Pops top value off the stack" << endl;
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
            case 'G':
            {
                if (!cellStack.empty()) {
                    cout << "Top stack cell:" << endl;
                    cout << "               " << cellStack.back().c64 << endl;
                } else {
                    cout << "Stack is empty. Nothing to show." << endl;
                }
            }
            break;
            case 'S':
            {
                CellType newVal;
                while (!(cin >> newVal.c64)) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }

                cellStack.back() = newVal;
            }
            break;
            case 'P':
            {
                CellType newVal;
                while (!(cin >> newVal.c64)) {
                    cin.clear();
                    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                }

                cellStack.push_back(newVal);
            }
            break;
            case 'p':
                if (!cellStack.empty()) {
                    cellStack.pop_back();
                } else {
                    cout << "Stack is empty. Nothing to pop." << endl;
                }
            break;
            }
        }
    }
}
