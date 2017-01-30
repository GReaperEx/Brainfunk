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
: CVanillaState(size, count, wrapPtr, dynamicTape, onEOF, dataFile), storage({0})
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

void CLoveState::compilePreInst(std::ostream& output)
{
    output << "CellType storage = 0;" << endl;
    CVanillaState::compilePreInst(output);
}

void CLoveState::runInstruction(const BFinstr& instr)
{
    CVanillaState::runInstruction(instr);

    switch (instr.token)
    {
    case '(':
        if (getCell(curPtrPos).c64 != 0) {
            int depth = 1;
            //! Make sure the parenthesis it jumps to is the correct one, at the same level
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
        if (getCell(curPtrPos).c64 == 0) {
            int depth = 1;
            //! Make sure the parenthesis it jumps to is the correct one, at the same level
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
    case '~':
        for (;;) {
            char token = getCode(++IP).token;
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
}

void CLoveState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    switch (instr.token)
    {
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
