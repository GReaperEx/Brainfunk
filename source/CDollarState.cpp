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

#include "CDollarState.h"

using namespace std;

CDollarState::CDollarState(int size, int count, bool wrapPtr, bool dynamicTape, const string& dataFile)
: IBasicState(size, count, wrapPtr, dynamicTape, dataFile), curPtrPos(0), IP(0)
{}

CDollarState::~CDollarState()
{}

void CDollarState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces
    int parenCount = 0;

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
        case '#':
        case '$':
        case ':':
        case ';':
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
        case '(':
            ++parenCount;
            instructions.push_back(BFinstr(c));
        break;
        case ')':
            --parenCount;
            instructions.push_back(BFinstr(c));
        break;
        }
    }
    if (bracesCount != 0) {
        throw runtime_error("Opening and closing braces don't match.");
    } else if (parenCount != 0) {
        throw runtime_error("Opening and closing parenthesis don't match.");
    }
}

void CDollarState::run()
{
    IP = 0;
    while (IP < instructions.size()) {
        switch (instructions[IP].token)
        {
        case '>':
            curPtrPos += instructions[IP].repeat;
        break;
        case '<':
            curPtrPos -= instructions[IP].repeat;
        break;
        case '+':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 += instructions[IP].repeat;
            setCell(curPtrPos, temp);
        }
        break;
        case '-':
        {
            CellType temp = getCell(curPtrPos);
            temp.c64 -= instructions[IP].repeat;
            setCell(curPtrPos, temp);
        }
        break;
        case '.':
        {
            CellType temp = getCell(curPtrPos);
            cout.put(temp.c8);
        }
        break;
        case ',':
        {
            int c = cin.get();
            CellType temp = { 0 };

            temp.c8 = (char)c;
            setCell(curPtrPos, temp);
        }
        break;
        case '[':
            if (getCell(curPtrPos).c64 == 0) {
                int depth = 1;
                //! Make sure the brace it jumps to is the correct one, at the same level
                while (depth > 0) {
                    ++IP;
                    char token = instructions[IP].token;
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
                    char token = instructions[IP].token;
                    if (token == '[') {
                        --depth;
                    } else if (token == ']') {
                        ++depth;
                    }
                }
            }
        break;
        case '#':
            cellStack.push_back(getCell(curPtrPos));
        break;
        case '$':
            if (!cellStack.empty()) {
                cellStack.pop_back();
            }
        break;
        case ':':
        {
            CellType temp = getCell(curPtrPos);
            cout << temp.c64;
        }
        break;
        case ';':
        {
            CellType temp;
            cin >> temp.c64;
            setCell(curPtrPos, temp);
        }
        break;
        case '(':
            instructions[IP].repeat = cellStack.back().c32 & 0x7FFFFFFF;
            if (instructions[IP].repeat == 0) {
                int depth = 1;
                while (depth > 0) {
                    ++IP;
                    char token = instructions[IP].token;
                    if (token == '(') {
                        ++depth;
                    } else if (token == ')') {
                        --depth;
                    }
                }
            }
        break;
        case ')':
        {
            int depth = 1;
            int startIP = IP;

            while (depth > 0) {
                --IP;
                char token = instructions[IP].token;
                if (token == '(') {
                    --depth;
                } else if (token == ')') {
                    ++depth;
                }
            }

            if (--instructions[IP].repeat <= 0) {
                IP = startIP;
            }
        }
        break;
        }

        ++IP;
    }
}

void CDollarState::compile(ostream& output)
{
    output << "#include <stdio.h>" << endl;
    output << "#include <stdint.h>" << endl;
    output << "#include <stdlib.h>" << endl;
    output << "#include <inttypes.h>" << endl;

    switch (getCellSize())
    {
    case 1:
        output << "typedef uint8_t CellType;" << endl;
    break;
    case 2:
        output << "typedef uint16_t CellType;" << endl;
    break;
    case 4:
        output << "typedef uint32_t CellType;" << endl;
    break;
    case 8:
        output << "typedef uint64_t CellType;" << endl;
    break;
    }

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

    if (isDynamic()) {
        output << "void* incReallocPtr(void* p, int* size, int index) {" << endl;
        output << "p = realloc(p, (index+1)*sizeof(CellType));" << endl;
        output << "if (!p) {" << endl;
        output << "fputs(\"Error: Out of memory!\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
        output << "memset((char*)p+*size*sizeof(CellType), 0, ((index+1)-*size)*sizeof(CellType));" << endl;
        output << "*size = index+1;" << endl;
        output << "return p;" << endl;
        output << "}" << endl;
    } else if (!wrapsPointer()) {
        output << "void incError() {" << endl;
        output << "fputs(\"Error: Tried to increment pointer beyond upper bound.\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
        output << "void decError() {" << endl;
        output << "fputs(\"Error: Tried to decrement pointer beyond lower bound.\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
    }

    if (!initData.empty()) {
        output << "const CellType datArray[] = { ";
        for (CellType cell : initData) {
            output << "0x" << hex << cell.c64 << ',';
        }
        output << " };" <<  endl;
    }
    if (!cellStack.empty()) {
        output << "const CellType stackData[] = { ";
        for (CellType cell : cellStack) {
            output << "0x" << hex << cell.c64 << ',';
        }
        output << " };" << endl;
    }

    output << "int main() {" << endl;
    output << "CellType* p = calloc(" << max(getCellCount(), (int)initData.size()) << ", sizeof(CellType));" << endl;
    output << "int index = 0;" << endl;
    output << "int size = " << max(getCellCount(), (int)initData.size()) << ';' << endl;

    output << "CellType* pS = NULL;" << endl;
    output << "int sIndex = -1;" << endl;
    output << "int sSize = 0;" << endl;
    output << "CellType temp;" << endl;


    if (!initData.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(datArray)/sizeof(datArray[0])); i++) {" << endl;
        output << "p[i] = datArray[i];" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }
    if (!cellStack.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(stackData)/sizeof(stackData[0]); i++) {" << endl;
        output << "pS = pushStack(pS, &sSize, &sIndex, stackData[i]);" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }

    for (auto it = instructions.begin(); it != instructions.end(); it++) {
        int repeat = it->repeat;
        switch (it->token)
        {
        case '>':
            if (wrapsPointer()) {
                output << "index = (index + " << repeat << ") % " << getCellCount() << ';' << endl;
            } else {
                output << "index += "<< repeat << ';' << endl;
                output << "if (index >= size) {" << endl;
                if (isDynamic()) {
                    output << "p = incReallocPtr(p, &size, index);" << endl;
                } else {
                    output << "incError();" << endl;
                }
                output << "}" << endl;
            }
        break;
        case '<':
            output << "index -= " << repeat << ';' << endl;
            output << "if (index < 0) {" << endl;
            if (wrapsPointer()) {
                output << "index = " << getCellCount() << " + index % " << getCellCount() << ';' << endl;
            } else {
                output << "decError();" << endl;
            }
            output << "}" << endl;
        break;
        case '+':
            output << "p[index] += " << repeat << ';' << endl;
        break;
        case '-':
            output << "p[index] -= " << repeat << ';' << endl;
        break;
        case '.':
            output << "putchar(p[index]);" << endl;
        break;
        case ',':
            output << "p[index] = getchar();" << endl;
        break;
        case '[':
            output << "while (p[index]) {" << endl;
        break;
        case ']':
            output << "}" << endl;
        break;
        case '#':
            output << "pS = pushStack(pS, &sSize, &sIndex, p[index]);" << endl;
        break;
        case '$':
            output << "p[index] = popStack(pS, &sIndex);" << endl;
        break;
        case ':':
            output << "printf(\"%\" PRIu" << getCellSize()*8 << ", p[index]);" << endl;
        break;
        case ';':
            output << "while (scanf(\"%\" SCNu" << getCellSize()*8 << ", &p[index]) != 1) {" << endl;
            output << "(void) getchar();" << endl;
            output << "}" << endl;
        break;
        case '(':
            output << "{" << endl;
            output << "CellType localLimit = peekStack(pS, sIndex);" << endl;
            output << "CellType localI;" << endl;
            output << "for (localI = 0; localI < localLimit; localI++) {" << endl;
        break;
        case ')':
            output << "}" << endl;
            output << "}" << endl;
        break;
        }
    }

    output << "free(p);" << endl;
    output << "}" << endl;
}
