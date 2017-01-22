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

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdint>

#define VERSION "0.1a"

using namespace std;

class bfState
{
public:
    bfState(int size, int count, bool wrapPtr) {
        if (size != 1 && size != 2 && size != 4 && size != 8) {
            throw runtime_error("Invalid cell size. Only 1, 2, 4 and 8 are supported.");
        }
        if (count <= 0) {
            throw runtime_error("Invalid cell count. Must be greater than zero.");
        } else if (count < 9999) {
            cerr << "Warning: Having less than 9999 cells isn't considered \"nice\"." << endl;
        }

        cellSize = size;
        cellCount = count;
        pool = new char[cellSize*cellCount];
        memset(pool, 0, cellSize*cellCount);

        curPtrPos = 0;
        IP = 0;
        ptrWrap = wrapPtr;
    }

    ~bfState() {
        delete [] pool;
    }

    void wrapPointer(bool justDOIT) {
        ptrWrap = justDOIT;
    }

    void translate(istream& input) {
        char c;
        int bracesCount = 0;

        instructions.clear();

        while (input.get(c)) {
            switch (c)
            {
            case '>':
            case '<':
            case '+':
            case '-':
                if (!instructions.empty() && instructions.back().getToken() == c) {
                    instructions.back().incr();
                } else {
                    instructions.push_back(BFinstruction(*this, c));
                }
            break;
            case '.':
            case ',':
                instructions.push_back(BFinstruction(*this, c));
            break;
            case '[':
                ++bracesCount;
                instructions.push_back(BFinstruction(*this, c));
            break;
            case ']':
                --bracesCount;
                instructions.push_back(BFinstruction(*this, c));
            break;
            }
        }
        if (bracesCount != 0) {
            throw runtime_error("Opening and closing braces don't match.");
        }
    }

    void run() {
        IP = 0;
        while (IP < instructions.size()) {
            instructions[IP].run();
            ++IP;
        }
    }

    void compile(ostream& output) {
        IP = 0;
        output << "#include <stdio.h>" << endl;
        output << "#include <stdint.h>" << endl;
        output << "#include <stdlib.h>" << endl;
        output << "int main() {" << endl;
        switch (cellSize)
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
        output << "p = calloc(" << cellCount << ", " << cellSize << ");" << endl;
        output << "int index = 0;" << endl;

        while (IP < instructions.size()) {
            instructions[IP].compile(output);
            ++IP;
        }

        output << "free(p);" << endl;
        output << "}" << endl;
    }

private:
    char* pool;
    int cellSize;
    int cellCount;

    int curPtrPos;
    unsigned IP;
    bool ptrWrap; //! Wrap the pointer around instead of generating error message

    class BFinstruction
    {
    public:
        BFinstruction(bfState& itsParent, char tok)
        : token(tok), repeat(1), parent(itsParent)
        {}

        char getToken() const {
            return token;
        }

        int getRepeat() const {
            return repeat;
        }
        void incr() {
            ++repeat;
        }

        void run() {
            int c;

            switch (token)
            {
            case '>':
                parent.curPtrPos += repeat;
                if (parent.curPtrPos >= parent.cellCount) {
                    if (parent.ptrWrap) {
                        parent.curPtrPos %= parent.cellCount;
                    } else {
                        throw runtime_error("Pointer was incremented too much.");
                    }
                }
            break;
            case '<':
                parent.curPtrPos -= repeat;
                if (parent.curPtrPos < 0) {
                    if (parent.ptrWrap) {
                        parent.curPtrPos = parent.cellCount + parent.curPtrPos % parent.cellCount;
                    } else {
                        throw runtime_error("Pointer was decremented too much.");
                    }
                }
            break;
            case '+':
                switch (parent.cellSize)
                {
                case 1:
                    ((uint8_t*)parent.pool)[parent.curPtrPos] += repeat;
                break;
                case 2:
                    ((uint16_t*)parent.pool)[parent.curPtrPos] += repeat;
                break;
                case 4:
                    ((uint32_t*)parent.pool)[parent.curPtrPos] += repeat;
                break;
                case 8:
                    ((uint64_t*)parent.pool)[parent.curPtrPos] += repeat;
                break;
                }
            break;
            case '-':
                switch (parent.cellSize)
                {
                case 1:
                    ((uint8_t*)parent.pool)[parent.curPtrPos] -= repeat;
                break;
                case 2:
                    ((uint16_t*)parent.pool)[parent.curPtrPos] -= repeat;
                break;
                case 4:
                    ((uint32_t*)parent.pool)[parent.curPtrPos] -= repeat;
                break;
                case 8:
                    ((uint64_t*)parent.pool)[parent.curPtrPos] -= repeat;
                break;
                }
            break;
            case '.':
                switch (parent.cellSize)
                {
                case 1:
                    cout.put((char)((uint8_t*)parent.pool)[parent.curPtrPos]);
                break;
                case 2:
                    cout.put((char)((uint16_t*)parent.pool)[parent.curPtrPos]);
                break;
                case 4:
                    cout.put((char)((uint32_t*)parent.pool)[parent.curPtrPos]);
                break;
                case 8:
                    cout.put((char)((uint64_t*)parent.pool)[parent.curPtrPos]);
                break;
                }
            break;
            case ',':
                c = cin.get();
                switch (parent.cellSize)
                {
                case 1:
                    ((uint8_t*)parent.pool)[parent.curPtrPos] = c;
                break;
                case 2:
                    ((uint16_t*)parent.pool)[parent.curPtrPos] = c;
                break;
                case 4:
                    ((uint32_t*)parent.pool)[parent.curPtrPos] = c;
                break;
                case 8:
                    ((uint64_t*)parent.pool)[parent.curPtrPos] = c;
                break;
                }
            break;
            case '[':
                if ((parent.cellSize == 1 && ( (uint8_t*)parent.pool)[parent.curPtrPos] == 0) ||
                    (parent.cellSize == 2 && ((uint16_t*)parent.pool)[parent.curPtrPos] == 0) ||
                    (parent.cellSize == 4 && ((uint32_t*)parent.pool)[parent.curPtrPos] == 0) ||
                    (parent.cellSize == 8 && ((uint64_t*)parent.pool)[parent.curPtrPos] == 0)) {
                    int depth = 1;
                    while (depth > 0) {
                        ++parent.IP;
                        char token = parent.instructions[parent.IP].getToken();
                        if (token == '[') {
                            ++depth;
                        } else if (token == ']') {
                            --depth;
                        }
                    }
                }
            break;
            case ']':
                if ((parent.cellSize == 1 && ( (uint8_t*)parent.pool)[parent.curPtrPos] != 0) ||
                    (parent.cellSize == 2 && ((uint16_t*)parent.pool)[parent.curPtrPos] != 0) ||
                    (parent.cellSize == 4 && ((uint32_t*)parent.pool)[parent.curPtrPos] != 0) ||
                    (parent.cellSize == 8 && ((uint64_t*)parent.pool)[parent.curPtrPos] != 0)) {
                    int depth = 1;
                    while (depth > 0) {
                        --parent.IP;
                        char token = parent.instructions[parent.IP].getToken();
                        if (token == '[') {
                            --depth;
                        } else if (token == ']') {
                            ++depth;
                        }
                    }
                }
            break;
            }
        }

        void compile(ostream& output) {
            switch (token)
            {
            case '>':
                if (parent.ptrWrap) {
                    output << "index = (index + " << repeat << ") % " << parent.cellCount << ';' << endl;
                } else {
                    output << "index += " << repeat << ';' << endl;
                }
            break;
            case '<':
                output << "index -= " << repeat << ';' << endl;
                if (parent.ptrWrap) {
                    output << "if (index < 0) {" << endl;
                    output << "index = " << parent.cellCount << " + index % " << parent.cellCount << ';' << endl;
                    output << "}" << endl;
                }
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

    private:
        char token;
        int repeat;

        bfState& parent;
    };
    vector<BFinstruction> instructions;
};

int main(int argc, char* argv[])
{
    int cellSize = 1;
    int cellCount = 32768; //! More than 30k, aligned for possible optimization when compiling BF code
    bool wrapPtr = false;
    bool compile = false;
    string output_file = "a.out";
    string input_file = "";

    for (int i = 1; i < argc; i++) {
        string temp(argv[i]);
        if (temp == "-h" || temp == "--help") {
            cout << "Usage:" << endl;
            cout << "    bfk [options] input_file" << endl;
            cout << "Options:" << endl;
            cout << "    -h, --help       ; Print this helpful message and exit" << endl;
            cout << "    -v, --version    ; Print program version and exit" << endl;
            cout << "    --cell-size=X    ; Sets cell size, only accepts 1, 2, 4 and 8 (Default=1)" << endl;
            cout << "    --cell-count=X   ; Sets amount of available cells (Default=32768)" << endl;
            cout << "    --wrap-pointer   ; Confines the memory pointer between bounds" << endl;
            cout << "    -c, --compile    ; Compiles BF code into native binary, by using gcc" << endl;
            cout << "    -o X, --output=X ; For compiling only (Default=\"a.out\")" << endl;
            exit(0);
        } else if (temp == "-v" || temp == "--version") {
            cout << "Copyright (C) 2017, GReaperEx(Marios F.)" << endl;
            cout << "Brainfunk v" VERSION << endl;
            exit(0);
        } else if (strncmp(argv[i], "--cell-size=", 12) == 0) {
            int temp;
            try {
                temp = stoi(string(&(argv[i][12])));
            } catch (invalid_argument& e) {
                temp = 0;
            }
            cellSize = temp;
        } else if (strncmp(argv[i], "--cell-count=", 13) == 0) {
            int temp;
            try {
                temp = stoi(string(&(argv[i][13])));
            } catch (invalid_argument& e) {
                temp = 0;
            }
            cellCount = temp;
        } else if (temp == "--wrap-pointer") {
            wrapPtr = true;
        } else if (temp == "-c" || temp == "--compile") {
            compile = true;
        } else if (temp == "-o") {
            if (i+1 < argc) {
                output_file = argv[++i];
            } else {
                cerr << "Error: Expected output file after \"-o\" option." << endl;
                exit(-1);
            }
        } else if (strncmp(argv[i], "--output=", 9) == 0) {
            if (!getline(stringstream(&argv[i][9]), output_file) || output_file.empty()) {
                cerr << "Error: Expected output file after \"--output=\" option." << endl;
                exit(-1);
            }
        } else {
            input_file = temp;
        }
    }

    try {
        if (input_file.empty()) {
            throw runtime_error("No input file was given.");
        }
        //cout << "abs" << endl;
        bfState myBF(cellSize, cellCount, wrapPtr);

        ifstream inputStream(input_file);
        if (!inputStream.is_open()) {
            throw runtime_error("Unable to open "+input_file+" for reading.");
        }
        myBF.translate(inputStream);
        inputStream.close();

        if (compile) {
            string tempFile = output_file+".c";
            ofstream outputStream(tempFile);
            if (!outputStream.is_open()) {
                throw runtime_error("Unable to open "+tempFile+" for writing.");
            }
            myBF.compile(outputStream);
            outputStream.close();

            if (system(("gcc -O3 -s -o "+output_file+" "+tempFile).c_str()) != 0) {
                throw runtime_error("Wasn't able to compile requested code.");
            }
            remove(tempFile.c_str());
        } else {
            myBF.run();
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
