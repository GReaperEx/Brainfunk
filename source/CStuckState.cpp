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

CStuckState::CStuckState(int size, int count, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: CVanillaState(size, count, false, dynamicTape, onEOF, dataFile, debug)
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

void CStuckState::compilePreMain(std::ostream& output)
{
    CVanillaState::compilePreMain(output);

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
}

void CStuckState::runInstruction(const BFinstr& instr)
{
    switch (instr.token)
    {
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
    {
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
        --IP;
    }
    break;
    }
}

void CStuckState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    int repeat = instr.repeat;
    switch (instr.token)
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
        if (eofPolicy == NOP) {
            output << "{" << endl;
            output << "CellType tempC = 1;" << endl;
            output << "getInput(&tempC);" << endl;
            output << "if (tempC != 1) {" << endl;
        }
        output << "pushStack(&p, &size, &index, getchar());" << endl;
        if (eofPolicy == NOP) {
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

