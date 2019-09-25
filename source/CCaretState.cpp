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

#include "CCaretState.h"

using namespace std;

CCaretState::CCaretState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const string& dataFile, bool debug)
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile, debug)
{}

CCaretState::~CCaretState()
{}

void CCaretState::translate(istream& input)
{
    char c;
    int bracesCount = 0; //! Counter to check for unbalanced braces
    int parenCount = 0;

    instructions.clear();

    while (input.get(c)) {
        switch (c)
        {
        case '>':
        case '<':
        case '+':
        case '-':
        case '{':
        case '}':
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

void CCaretState::compilePreMain(std::ostream& output)
{
    CVanillaState::compilePreMain(output);

    output << "CellType* indirectGet(CellType** p, int* size, int index) {" << endl;
    output << "if (index < 0) {" << endl;
    if (ptrWrap) {
        output << "index = " << cellCount << " + index % " << cellCount << ';' << endl;
    } else {
        output << "decError();" << endl;
    }
    output << "} else if (index >= *size) {" << endl;
    if (ptrWrap) {
        output << "index %= " << cellCount << ';' << endl;
    } else {
        if (dynamic) {
            output << "*p = incReallocPtr(*p, size, index);" << endl;
        } else {
            output << "incError();" << endl;
        }
    }
    output << "}" << endl;
    output << "return (*p + index);" << endl;
    output << "}" << endl;
}

void CCaretState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
    case '{':
    {
        CellType ptr = getCell(curPtrPos);
        CellType temp = getCell(ptr.c32);
        temp.c64 -= instr.repeat;
        setCell(ptr.c32, temp);
    }
    break;
    case '}':
    {
        CellType ptr = getCell(curPtrPos);
        CellType temp = getCell(ptr.c32);
        temp.c64 += instr.repeat;
        setCell(ptr.c32, temp);
    }
    break;
    case '(':
        if (getCell(getCell(curPtrPos).c32).c64 == 0) {
            int depth = 1;
            //! Make sure the brace it jumps to is the correct one, at the same level
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
        if (getCell(getCell(curPtrPos).c32).c64 != 0) {
            int depth = 1;
            //! Make sure the brace it jumps to is the correct one, at the same level
            while (depth > 0) {
                --IP;
                char token = getCode(IP).token;
                if (token == '(') {
                    --depth;
                } else if (token == ')') {
                    ++depth;
                }
            }
        }
    break;
    }
}

void CCaretState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    CVanillaState::compileInstruction(output, instr);

    switch (instr.token)
    {
    case '{':
        output << "*indirectGet(&p, &size, p[index]) -= " << instr.repeat << ';' << endl;
    break;
    case '}':
        output << "*indirectGet(&p, &size, p[index]) += " << instr.repeat << ';' << endl;
    break;
    case '(':
        output << "while (*indirectGet(&p, &size, p[index])) {" << endl;
    break;
    case ')':
        output << "}" << endl;
    break;
    }
}
