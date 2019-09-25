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

#include "CVanillaState.h"

#include <limits>

#include <signal.h>

CVanillaState* CVanillaState::state = nullptr;

CVanillaState::CVanillaState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug)
: curPtrPos(0), IP(0), doDebug(debug), dbgPaused(true)
{
    if (size != 1 && size != 2 && size != 4 && size != 8) {
        throw std::runtime_error("Invalid cell size. Only 1, 2, 4 and 8 are supported.");
    }
    if (count <= 0 && !dynamicTape) {
        throw std::runtime_error("Invalid cell count. Must be greater than zero.");
    } else if (count < 9999 && !dynamicTape) {
        std::cerr << "Warning: Having less than 9999 cells isn't considered \"nice\"." << std::endl;
    }
    if (wrapPtr && wrapPtr == dynamicTape) {
        throw std::runtime_error("Cannot have both a dynamic tape and a wrap-around pointer.");
    }

    cellSize = size;

    if (dynamicTape) {
        cellCount = 10000;
    } else {
        cellCount = count;
    }

    ptrWrap = wrapPtr;
    dynamic = dynamicTape;
    eofPolicy = onEOF;

    tape = calloc(cellCount, cellSize);
    if (tape == nullptr) {
        throw std::runtime_error("There's not enough memory available!");
    }

    if (!dataFile.empty()) {
        std::ifstream input(dataFile);
        if (!input.is_open()) {
            throw std::runtime_error("Unable to open requested data file.");
        }

        parseData(input);
    }

    if (doDebug) {
        CVanillaState::state = this;
        signal(SIGINT, CVanillaState::signalHandle);
    }
}

CVanillaState::~CVanillaState()
{
    free(tape);
}

void CVanillaState::translate(std::istream& input)
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
        throw std::runtime_error("Opening and closing braces don't match.");
    }

    //! Appends collected data to tape
    for (unsigned i = 0; i < initData.size(); i++) {
        setCell(i, initData[i]);
    }
}

void CVanillaState::run()
{
    IP = 0;
    keepRunning = hasInstructions();

    while (keepRunning) {
        if (doDebug) {
            runDebug();
        }

        runInstruction(getCode(IP));
        ++IP;
    }
}

void CVanillaState::compile(std::ostream& output)
{
    compilePreMain(output);
    output << "int main() {" << std::endl;
    compilePreInst(output);

    for (auto it = instructions.begin(); it != instructions.end(); it++) {
        compileInstruction(output, *it);
    }

    compileCleanup(output);
    output << "}" << std::endl;
}

const IBasicState::CellType CVanillaState::getCell(int cellIndex)
{
    CellType toReturn = { 0 };

    examineIndex(cellIndex);

    switch (cellSize)
    {
    case 1:
        toReturn.c8 = ((uint8_t*)tape)[cellIndex];
    break;
    case 2:
        toReturn.c16 = ((uint16_t*)tape)[cellIndex];
    break;
    case 4:
        toReturn.c32 = ((uint32_t*)tape)[cellIndex];
    break;
    case 8:
        toReturn.c64 = ((uint64_t*)tape)[cellIndex];
    break;
    }

    return toReturn;
}

void CVanillaState::setCell(int cellIndex, const CellType& newValue)
{
    examineIndex(cellIndex);

    switch (cellSize)
    {
    case 1:
        ((uint8_t*)tape)[cellIndex] = newValue.c8;
    break;
    case 2:
        ((uint16_t*)tape)[cellIndex] = newValue.c16;
    break;
    case 4:
        ((uint32_t*)tape)[cellIndex] = newValue.c32;
    break;
    case 8:
        ((uint64_t*)tape)[cellIndex] = newValue.c64;
    break;
    }
}

void CVanillaState::parseData(std::istream& input)
{
    char c;

    while (input.get(c)) {
        CellType temp = { 0 };
        if (c == '\\') {
            if (!input.get(c)) {
                throw std::runtime_error("Data: Expected symbol after \'\\\'.");
            }
            switch (c)
            {
            case 'a':
                temp.c8 = '\a';
            break;
            case 'b':
                temp.c8 = '\b';
            break;
            case 'f':
                temp.c8 = '\f';
            break;
            case 'n':
                temp.c8 = '\n';
            break;
            case 'r':
                temp.c8 = '\r';
            break;
            case 't':
                temp.c8 = '\t';
            break;
            case 'v':
                temp.c8 = '\v';
            break;
            case '\\':
                temp.c8 = '\\';
            break;
            case '\'':
                temp.c8 = '\'';
            break;
            case '\"':
                temp.c8 = '\"';
            break;
            case '?':
                temp.c8 = '?';
            break;
            case 'x':
                for (int i = 0; i < cellSize*2; i++) {
                    int c = tolower(input.peek());
                    if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) {
                        break;
                    }
                    c = input.get();

                    temp.c64 <<= 4;
                    if (c >= 'a' && c <= 'f') {
                        temp.c64 += (c - 'a') + 10;
                    } else {
                        temp.c64 += (c - '0');
                    }
                }
            break;
            default:
                if (!(c >= '0' && c <= '7')) {
                    throw std::runtime_error("Data: Unexpected symbol after \'\\\'.");
                }
                for (int i = 0; i < 3; i++) {
                    int c = tolower(input.peek());
                    if (!(c >= '0' && c <= '7')) {
                        break;
                    }
                    c = input.get();

                    temp.c64 <<= 3;
                    temp.c64 += (c - '0');
                }
            }
        } else {
            temp.c8 = c;
        }
        initData.push_back(temp);
    }
}

bool CVanillaState::userInput(uint8_t& c)
{
    char temp;
    if (!std::cin.get(temp)) {
        switch (eofPolicy)
        {
        case RETM1:
            c = -1;
        break;
        case RET0:
            c = 0;
        break;
        case NOP:
            return false;
        break;
        case ABORT:
            throw std::runtime_error("Encountered EOF while processing input.");
        break;
        }
        return true;
    }
    c = temp;
    return true;
}

void CVanillaState::examineIndex(int& cellIndex)
{
    if (cellIndex < 0) {
        if (ptrWrap) {
            cellIndex = cellCount - cellIndex % cellCount;
        } else {
            throw std::runtime_error("Pointer was decremented too much.");
        }
    }

    if (dynamic) {
        //! Either extend the tape enough or die trying
        while (cellIndex >= cellCount) {
            int oldCount = cellCount;
            cellCount += cellCount/2 + 1;

            tape = realloc(tape, cellCount*cellSize);
            if (tape == nullptr) {
                throw std::runtime_error("There's not enough memory available!");
            }

            memset((char*)tape + oldCount*cellSize, 0, cellCount*cellSize - oldCount*cellSize);
        }
    } else {
        if (cellIndex >= cellCount) {
            if (ptrWrap) {
                cellIndex %= cellCount;
            } else {
                throw std::runtime_error("Pointer was incremented too much.");
            }
        }
    }
}

void CVanillaState::compilePreMain(std::ostream& output)
{
    using std::endl;

    output << "#include <stdio.h>" << endl;
    output << "#include <stdint.h>" << endl;
    output << "#include <stdlib.h>" << endl;
    output << "#include <string.h>" << endl;

    switch (cellSize)
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

    if (dynamic) {
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
    }
    if (!ptrWrap) {
        output << "void incError() {" << endl;
        output << "fputs(\"Error: Tried to increment pointer beyond upper bound.\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
        output << "void decError() {" << endl;
        output << "fputs(\"Error: Tried to decrement pointer beyond lower bound.\\n\", stderr);" << endl;
        output << "exit(-1);" << endl;
        output << "}" << endl;
    }

    output << "void getInput(CellType* dst) {" << endl;
    output << "int temp = getchar();" << endl;
    output << "if (temp == EOF) {" << endl;
    switch (eofPolicy)
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
            output << "0x" << std::hex << cell.c64 << ',';
        }
        output << '\b' << " };" << endl;
    }
}

void CVanillaState::compilePreInst(std::ostream& output)
{
    using std::endl;
    using std::max;

    output << "CellType* p = calloc(" << max(cellCount, (int)initData.size()) << ", sizeof(CellType));" << endl;
    output << "int index = 0;" << endl;
    output << "int size = " << max(cellCount, (int)initData.size()) << ';' << endl;

    if (!initData.empty()) {
        output << "{" << endl;
        output << "int i;" << endl;
        output << "for (i = 0; i < sizeof(datArray)/sizeof(CellType); i++) {" << endl;
        output << "p[i] = datArray[i];" << endl;
        output << "}" << endl;
        output << "}" << endl;
    }
}

void CVanillaState::compileCleanup(std::ostream& output)
{
    output << "free(p);" << std::endl;
}

void CVanillaState::runInstruction(const BFinstr& instr)
{
    switch (instr.token)
    {
    case '>':
        curPtrPos += instr.repeat;
    break;
    case '<':
        curPtrPos -= instr.repeat;
    break;
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
        CellType temp = getCell(curPtrPos);
        std::cout.put(temp.c8);
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
        if (getCell(curPtrPos).c64 != 0) {
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
        }
    break;
    }

    if (IP + 1 >= instructions.size()) {
        keepRunning = false;
    }
}

void CVanillaState::compileInstruction(std::ostream& output, const BFinstr& instr)
{
    using std::endl;

    int repeat = instr.repeat;
    switch (instr.token)
    {
    case '>':
        if (ptrWrap) {
            output << "index = (index + " << repeat << ") % " << cellCount << ';' << endl;
        } else {
            output << "index += "<< repeat << ';' << endl;
            output << "if (index >= size) {" << endl;
            if (dynamic) {
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
        if (ptrWrap) {
            output << "index = " << cellCount << " + index % " << cellCount << ';' << endl;
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
        output << "getInput(&p[index]);" << endl;
    break;
    case '[':
        output << "while (p[index]) {" << endl;
    break;
    case ']':
        output << "}" << endl;
    break;
    }

    if (IP >= instructions.size()) {
        keepRunning = false;
    }
}

CVanillaState::BFinstr& CVanillaState::getCode(int ip)
{
    return instructions[ip];
}

void CVanillaState::runDebug()
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
