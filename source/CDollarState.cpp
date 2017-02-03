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

CDollarState::CDollarState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug)
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

void CDollarState::compilePreMain(std::ostream& output)
{
    output << "#include <inttypes.h>" << endl;

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

void CDollarState::compilePreInst(std::ostream& output)
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

void CDollarState::compileCleanup(std::ostream& output)
{
    CVanillaState::compileCleanup(output);
    output << "free(pS);" << endl;
}

void CDollarState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
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
        CellType temp = getCell(curPtrPos);
        if (!(cin >> temp.c64)) {
            switch (eofPolicy)
            {
            case IBasicState::RETM1:
                temp.c64 = -1;
            break;
            case IBasicState::RET0:
                temp.c64 = 0;
            break;
            case IBasicState::NOP:
                temp = getCell(curPtrPos);
            break;
            case IBasicState::ABORT:
                throw runtime_error("Encountered EOF while processing input.");
            break;
            }
        }
        setCell(curPtrPos, temp);
    }
    break;
    case '(':
        getCode(IP).repeat = cellStack.back().c32 & 0x7FFFFFFF;
        if (getCode(IP).repeat == 0) {
            int depth = 1;
            while (depth > 0) {
                ++IP;
                char token = getCode(IP).token;
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
            char token = getCode(IP).token;
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
}

void CDollarState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    CVanillaState::compileInstruction(output, instr);

    switch (instr.token)
    {
    case '#':
        output << "pS = pushStack(pS, &sSize, &sIndex, p[index]);" << endl;
    break;
    case '$':
        output << "p[index] = popStack(pS, &sIndex);" << endl;
    break;
    case ':':
        output << "printf(\"%\" PRIu" << cellSize*8 << ", p[index]);" << endl;
    break;
    case ';':
        output << "while (scanf(\"%\" SCNu" << cellSize*8 << ", &p[index]) != 1) {" << endl;
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
