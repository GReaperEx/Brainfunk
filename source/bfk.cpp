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

#include "CVanillaState.h"
#include "CExtendedState.h"
#include "CExtended2State.h"
#include "CExtended3State.h"
#include "CLoveState.h"
#include "CStackedState.h"
#include "CBCDState.h"
#include "CStuckState.h"
#include "CJumpState.h"
#include "CDollarState.h"

#define VERSION "0.5.1"

using namespace std;

enum LangVariants { VANILLA, EXTENDED, EXTENDED2, EXTENDED3, LOVE, STACKED, BCD, STUCK, JUMP, DOLLAR };

int main(int argc, char* argv[])
{
    int cellSize = 1;
    int cellCount = 32768; //! More than 30k, aligned for possible optimization when compiling BF code
    bool wrapPtr = false;
    bool dynamic = false;

    LangVariants useVariant = VANILLA;

    bool compile = false;
    string output_file = "a.out";
    string input_file = "";
    string dataFile = "";

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
            cout << "    --dynamic-tape   ; Makes the \'tape\' grow dynamically, without limit" << endl;
            cout << "    -x, --extended   ; Uses \'Extended Brainfuck Type I\' instead of vanilla" << endl;
            cout << "    -x2, --extended2 ; Uses \'Extended Brainfuck Type II\' instead" << endl;
            cout << "    -x3, --extended3 ; Uses \'Extended Brainfuck Type III\' instead" << endl;
            cout << "    -j, --jump       ; Uses \'JumpFuck\' instead" << endl;
            cout << "    --love           ; Uses \'Brainlove\' instead" << endl;
            cout << "    --stacked        ; Uses \'Stacked Brainfuck\' instead" << endl;
            cout << "    --bcd            ; Uses \'BCDFuck\' instead" << endl;
            cout << "    --stuck          ; Uses \'Brainstuck\' instead" << endl;
            cout << "    --dollar         ; Uses \'Brainfuck$\' instead" << endl;
            cout << "    -c, --compile    ; Compiles BF code into native binary, if possible" << endl;
            cout << "    -o X, --output=X ; For compiling only (Default=\"a.out\")" << endl;
            cout << "    -d X, --data=X   ; Memory initialization data( ASCII file )" << endl;
            exit(0);
        } else if (temp == "-v" || temp == "--version") {
            cout << "Copyright (C) 2017, GReaperEx(Marios F.)" << endl;
            cout << "Brainfunk v" VERSION << endl;
            exit(0);
        } else if (strncmp(argv[i], "--cell-size=", 12) == 0) {
            if (!(stringstream(&argv[i][12]) >> cellSize)) {
                cellSize = 0; //! Have it get handled by bsState's constructor
            }
        } else if (strncmp(argv[i], "--cell-count=", 13) == 0) {
            if (!(stringstream(&argv[i][13]) >> cellCount)) {
                cellCount = 0;
            }
        } else if (temp == "--wrap-pointer") {
            wrapPtr = true;
        } else if (temp == "--dynamic-tape") {
            dynamic = true;
        } else if (temp == "-x" || temp == "--extended") {
            useVariant = EXTENDED;
        } else if (temp == "-x2" || temp == "--extended2") {
            useVariant = EXTENDED2;
        } else if (temp == "-x3" || temp == "--extended3") {
            useVariant = EXTENDED3;
        } else if (temp == "--love") {
            useVariant = LOVE;
        } else if (temp == "--stacked") {
            useVariant = STACKED;
        } else if (temp == "--bcd") {
            useVariant = BCD;
        } else if (temp == "--stuck") {
            useVariant = STUCK;
        } else if (temp == "--dollar") {
            useVariant = DOLLAR;
        } else if (temp == "-j" || temp == "--jump") {
            useVariant = JUMP;
        } else if (temp == "-c" || temp == "--compile") {
            compile = true;
        } else if (temp == "-o") {
            if (i+1 < argc) {
                if (output_file != "a.out") {
                    cerr << "Warning: Output file was set more than once. Ignoring previous value." << endl;
                }
                output_file = argv[++i];
            } else {
                cerr << "Error: Expected output file after \"-o\" option." << endl;
                exit(-1);
            }
        } else if (strncmp(argv[i], "--output=", 9) == 0) {
            if (output_file != "a.out") {
                cerr << "Warning: Output file was set more than once. Ignoring previous value." << endl;
            }
            if (!getline(stringstream(&argv[i][9]), output_file) || output_file.empty()) {
                cerr << "Error: Expected output file after \"--output=\" option." << endl;
                exit(-1);
            }
        } else if (temp == "-d") {
            if (i+1 < argc) {
                if (dataFile != "") {
                    cerr << "Warning: Data file was set more than once. Ignoring previous value." << endl;
                }
                dataFile = argv[++i];
            } else {
                cerr << "Error: Expected data file after \"-d\" option." << endl;
                exit(-1);
            }
        } else if (strncmp(argv[i], "--data=", 7) == 0) {
            if (dataFile != "") {
                cerr << "Warning: Data file was set more than once. Ignoring previous value." << endl;
            }
            if (!getline(stringstream(&argv[i][7]), dataFile) || dataFile.empty()) {
                cerr << "Error: Expected data file after \"--data=\" option." << endl;
                exit(-1);
            }
        } else {
            if (!input_file.empty()) {
                cerr << "Warning: Input file was set more than once. Ignoring previous value." << endl;
            }
            input_file = temp;
        }
    }

    try {
        if (input_file.empty()) {
            throw runtime_error("No input file was given.");
        }

        IBasicState* myBF;

        switch (useVariant)
        {
        case VANILLA:
            myBF = new CVanillaState(cellSize, cellCount, wrapPtr, dynamic, dataFile);
        break;
        case EXTENDED:
            myBF = new CExtendedState(cellSize, dataFile);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case EXTENDED2:
            myBF = new CExtended2State(cellSize, dataFile);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case EXTENDED3:
            myBF = new CExtended3State(cellSize, dataFile);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case LOVE:
            myBF = new CLoveState(cellSize, cellCount, wrapPtr, dynamic, dataFile);
        break;
        case STACKED:
            myBF = new CStackedState(cellSize, cellCount, wrapPtr, dynamic, dataFile);
        break;
        case BCD:
            myBF = new CBCDState(cellCount, wrapPtr, dynamic, dataFile);
            if (cellSize != 1) {
                cerr << "Warning: Custom cell size ignored. 8-bit supported only." << endl;
            }
        break;
        case STUCK:
            myBF = new CStuckState(cellSize, cellCount, dynamic, dataFile);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case JUMP:
            myBF = new CJumpState(cellSize, dataFile);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case DOLLAR:
            myBF = new CDollarState(cellSize, cellCount, wrapPtr, dynamic, dataFile);
        break;
        }

        ifstream inputStream(input_file);
        if (!inputStream.is_open()) {
            throw runtime_error("Unable to open "+input_file+" for reading.");
        }
        myBF->translate(inputStream);
        inputStream.close();

        if (compile) {
            string tempFile = output_file+".c";
            ofstream outputStream(tempFile);
            if (!outputStream.is_open()) {
                throw runtime_error("Unable to open "+tempFile+" for writing.");
            }
            myBF->compile(outputStream);
            outputStream.close();

            if (system(("gcc -O3 -s -o "+output_file+" "+tempFile).c_str()) != 0) {
                throw runtime_error("Wasn't able to compile requested code.");
            }
            remove(tempFile.c_str());
        } else {
            myBF->run();
        }
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
