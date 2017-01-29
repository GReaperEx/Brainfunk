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

 #include "CLoveState.h"

using namespace std;

CLoveState::CLoveState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile)
: IBasicState(size, count, wrapPtr, dynamicTape, onEOF, dataFile), curPtrPos(0), IP(0), storage({0})
{}

CLoveState::~CLoveState()
{}

void CLoveState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces
    int parenCount = 0; //! Counter for unbalanced parenthesis

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
        case '~':
            if (bracesCount == 0 && parenCount == 0) {
                throw runtime_error("Cannot break out of non-existent loop.");
            }
            instructions.push_back(BFinstr(c));
        break;
        case '.':
        case ',':
        case '$':
        case '!':
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

    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(i, initData[i]);
    }
}

void CLoveState::run()
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
        case '(':
            if (getCell(curPtrPos).c64 != 0) {
                int depth = 1;
                //! Make sure the parenthesis it jumps to is the correct one, at the same level
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
            if (getCell(curPtrPos).c64 == 0) {
                int depth = 1;
                //! Make sure the parenthesis it jumps to is the correct one, at the same level
                while (depth > 0) {
                    --IP;
                    char token = instructions[IP].token;
                    if (token == '(') {
                        --depth;
                    } else if (token == ')') {
                        ++depth;
                    }
                }
            }
        break;
        case '~':
            for (;;) {
                char token = instructions[++IP].token;
                if (token == ')' || token == ']') {
                    break;
                }
            }
        break;
        case '$':
            storage = getCell(curPtrPos);
        break;
        case '!':
            setCell(curPtrPos, storage);
        break;
        }

        ++IP;
    }
}

void CLoveState::compile(ostream& output)
{
    output << "#include <stdio.h>" << endl;
    output << "#include <string.h>" << endl;
    output << "#include <stdint.h>" << endl;
    output << "#include <stdlib.h>" << endl;

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

    output << "CellType storage = 0;";

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
        case '>':
            if (wrapsPointer()) {
                output << "index = (index + " << repeat << ") % " << getCellCount() << ';' << endl;
            } else {
                output << "index += "<< repeat << ';' << endl;
                output << "if (index >= size) {" << endl;
                output << "p = incReallocPtr(p, &size, index);" << endl;
                output << "}" << endl;
            }
        break;
        case '<':
            output << "index -= " << repeat << ';' << endl;
            output << "if (index < 0) {" << endl;
            output << "decError();" << endl;
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
            output << "getInput(&p[index]);" << endl;
        break;
        case '[':
            output << "while (p[index]) {" << endl;
        break;
        case ']':
            output << "}" << endl;
        break;
        case '(':
            output << "while (!p[index]) {" << endl;
        break;
        case ')':
            output << "}" << endl;
        break;
        case '~':
            output << "break;" << endl;
        break;
        case '$':
            output << "storage = p[index];" << endl;
        break;
        case '!':
            output << "p[index] = storage;" << endl;
        break;
        }
    }

    output << "free(p);" << endl;
    output << "}" << endl;
}

