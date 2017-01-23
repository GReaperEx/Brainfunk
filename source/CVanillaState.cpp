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

#include "CVanillaState.h"

using namespace std;

CVanillaState::CVanillaState(int size, int count, bool wrapPtr, bool dynamicTape)
: IBasicState(size, count, wrapPtr, dynamicTape), curPtrPos(0), IP(0)
{}

CVanillaState::~CVanillaState()
{}

void CVanillaState::translate(istream& input)
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
}

void CVanillaState::run()
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
        }

        ++IP;
    }
}

void CVanillaState::compile(ostream& output)
{
    output << "#include <stdio.h>" << endl;
    output << "#include <stdint.h>" << endl;
    output << "#include <stdlib.h>" << endl;

    if (isDynamic()) {
        output << "void* incReallocPtr(void* p, int* size, int index) {" << endl;
        output << "p = realloc(p, (index+1)*" << getCellSize() << ");" << endl;
        output << "if (!p) {" << endl;
        output << "fputs(\"Error: Out of memory!\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
        output << "memset((char*)p+*size*" << getCellSize() << ", 0, ((index+1)-*size)*" << getCellSize() << ");" << endl;
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

    output << "int main() {" << endl;
    switch (getCellSize())
    {
    case 1:
        output << "uint8_t* ";
    break;
    case 2:
        output << "uint16_t* ";
    break;
    case 4:
        output << "uint32_t* ";
    break;
    case 8:
        output << "uint64_t* ";
    break;
    }
    output << "p = calloc(" << getCellCount() << ", " << getCellSize() << ");" << endl;
    output << "int index = 0;" << endl;
    output << "int size = " << getCellCount() << ';' << endl;

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
        }
    }

    output << "free(p);" << endl;
    output << "}" << endl;
}
