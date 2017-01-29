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

#include "CStuckState.h"

using namespace std;

CStuckState::CStuckState(int size, int count, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile)
: IBasicState(size, count, false, dynamicTape, onEOF, dataFile), curPtrPos(0), IP(0)
{}

CStuckState::~CStuckState()
{}

void CStuckState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces

    instructions.clear();

    while (input.get(c)) {
        switch (c)
        {
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
        case '0':
        case ':':
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

void CStuckState::run()
{
    IP = 0;
    while (IP < instructions.size()) {
        switch (instructions[IP].token)
        {
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
            CellType temp = getCell(curPtrPos--);
            cout.put(temp.c8);
        }
        break;
        case ',':
        {
            CellType temp = { 0 };

            if (userInput(temp.c8)) {
                setCell(++curPtrPos, temp);
            }
        }
        break;
        case '0':
        {
            CellType temp { 0 };
            setCell(++curPtrPos, temp);
        }
        break;
        case ':':
        {
            CellType temp = getCell(curPtrPos--);
            temp = getCell(curPtrPos - temp.c32);
            setCell(++curPtrPos, temp);
        }
        break;
        case '[':
            if (getCell(curPtrPos--).c64 == 0) {
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
        {
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
            --IP;
        }
        break;
        }

        ++IP;
    }
}

void CStuckState::compile(ostream& output)
{
    output << "#include <stdio.h>" << endl;
    output << "#include <stdint.h>" << endl;
    output << "#include <stdlib.h>" << endl;
    output << "#include <string.h>" << endl;

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

    output << "void decError() {" << endl;
    output << "fputs(\"Error: Tried to decrement pointer beyond lower bound.\\n\", stderr);" << endl;
    output << "exit(-1);" << endl;
    output << "}" << endl;

    output << "void pushStack(CellType** stack, int* size, int* index, CellType newVal) {" << endl;
    output << "if (++*index >= *size) {" << endl;
    output << "*stack = incReallocPtr(*stack, size, *index);" << endl;
    output << "}" << endl;
    output << "(*stack)[*index] = newVal;" << endl;
    output << "}" << endl;

    output << "CellType popStack(CellType* stack, int* size, int* index) {" << endl;
    output << "if (*index < 0) {" << endl;
    output << "decError();" << endl;
    output << "}" << endl;
    output << "return stack[(*index)--];" << endl;
    output << "}" << endl;

    output << "void getInput(CellType* dst) {" << endl;
    output << "int temp = getchar();" << endl;
    output << "if (temp == EOF) {" << endl;
    switch (getEOFpolicy())
    {
    case RETM1:
        output << "*dst = -1;" << endl;
    break;
    case RET0:
        output << "*dst = 0;" << endl;
    break;
    case NOP:
    break;
    case ABORT:
        output << "fputs(\"Error: Encountered EOF while processing input.\", stderr);" << endl;
        output << "exit(-1);" << endl;
    break;
    }
    output << "} else {" << endl;
    output << "*dst = (CellType)temp;" << endl;
    output << "}" << endl;
    output << "}" << endl;

    if (!initData.empty()) {
        output << "const CellType datArray[] = { ";
        for (CellType cell : initData) {
            output << "0x" << hex << cell.c64 << ',';
        }
        output << '\b' << " };" << endl;
    }

    output << "int main() {" << endl;
    output << "CellType* p = calloc(" << max(getCellCount(), (int)initData.size()) << ", sizeof(CellType));" << endl;
    output << "int index = 0;" << endl;
    output << "int size = " << max(getCellCount(), (int)initData.size()) << ';' << endl;

    if (!initData.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(datArray)/sizeof(CellType); i++) {" << endl;
        output << "p[i] = datArray[i];" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }

    for (auto it = instructions.begin(); it != instructions.end(); it++) {
        int repeat = it->repeat;
        switch (it->token)
        {
        case '+':
            output << "p[index] += " << repeat << ';' << endl;
        break;
        case '-':
            output << "p[index] -= " << repeat << ';' << endl;
        break;
        case '.':
            output << "putchar(popStack(p, &size, &index));" << endl;
        break;
        case ',':
            if (getEOFpolicy() == NOP) {
                output << "{" << endl;
                output << "CellType tempC = 1;" << endl;
                output << "getInput(&tempC);" << endl;
                output << "if (tempC != 1) {" << endl;
            }
            output << "pushStack(&p, &size, &index, getchar());" << endl;
            if (getEOFpolicy() == NOP) {
                output << "}" << endl;
                output << "}" << endl;
            }
        break;
        case '[':
            output << "while (popStack(p, &size, &index)) {" << endl;
        break;
        case ']':
            output << "}" << endl;
        break;
        case '0':
            output << "pushStack(&p, &size, &index, 0);" << endl;
        break;
        case ':':
            output << "{" << endl;
            output << "CellType temp = popStack(p, &size, &index)];" << endl;
            output << "pushStack(&p, &size, &index, p[(uint32_t)(index - temp)]);" << endl;
            output << "}" << endl;
        break;
        }
    }

    output << "free(p);" << endl;
    output << "}" << endl;
}
