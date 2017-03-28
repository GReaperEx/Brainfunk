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

#include <getopt.h>

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
#include "CSelfmodState.h"
#include "CCaretState.h"
#include "CBitchanState.h"
#include "CCompressedState.h"
#include "CLollerState.h"
#include "CExtLollerState.h"
#include "CDrawState.h"

#define VERSION "0.9.0"

using namespace std;

enum LangVariants { VANILLA, EXTENDED, EXTENDED2, EXTENDED3, LOVE, STACKED, BCD, STUCK, JUMP,
                    DOLLAR, SELFMOD, CARET, BITCHAN, COMPRESSED, LOLLER, EXTLOLLER, DRAW };

const char shortOptions[] = "hvs:t:wye:co:d:ijx::b";

const option longOptions[] = {
    { "help",         no_argument,       0, 'h' },
    { "version",      no_argument,       0, 'v' },
    { "cell-size",    required_argument, 0, 's' },
    { "tape-size",    required_argument, 0, 't' },
    { "wrap-pointer", no_argument,       0, 'w' },
    { "dynamic-tape", no_argument,       0, 'y' },
    { "eof-action",   required_argument, 0, 'e' },
    { "compile",      no_argument,       0, 'c' },
    { "output",       required_argument, 0, 'o' },
    { "data",         required_argument, 0, 'd' },
    { "stdin",        no_argument,       0, 'i' },
    { "debug",        no_argument,       0, 'b' },
    { "lang",         required_argument, 0, 256 },
    { 0, 0, 0, 0 }
};

int main(int argc, char* argv[])
{
    int cellSize = 1;
    int cellCount = 32768; //! More than 30k, aligned for possible optimization when compiling BF code
    bool wrapPtr = false;
    bool dynamic = false;
    bool debug = false;

    LangVariants useVariant = VANILLA;
    IBasicState::ActionOnEOF onEOF = IBasicState::RETM1;

    bool compile = false;
    string output_file = "a.out";
    string input_file = "";
    string dataFile = "";

    bool useStdin = false;

    //! Parsing everything

    for (;;) {
        int longIndex;
        int op = getopt_long(argc, argv, shortOptions, longOptions, &longIndex);
        if (op == -1) {
            break;
        }

        switch (op)
        {
        case 'h':
            cout << "Usage:" << endl;
            cout << "  bfk [options] input_file" << endl;
            cout << "Options:" << endl;
            cout << "  -h, --help            ; Print this helpful message and exit" << endl;
            cout << "  -v, --version         ; Print program version and exit" << endl;
            cout << "  -b, --debug           ; Enables debug mode for the interpreter" << endl;
            cout << "  -s X, --cell-size=X   ; Sets cell size, only accepts 1, 2, 4 or 8 (Default=1)" << endl;
            cout << "  -t X, --tape-size=X   ; Sets amount of available cells (Default=32768)" << endl;
            cout << "  -w, --wrap-pointer    ; Confines the memory pointer between bounds" << endl;
            cout << "  -y, --dynamic-tape    ; Makes the \'tape\' grow dynamically, without limit" << endl;
            cout << "  -e X, --eof-action=X  ; Changes the default behavior when managing EOF" << endl;
            cout << "        -1              ; Returns -1 to the program (Default)" << endl;
            cout << "        0               ; Returns 0 to the program" << endl;
            cout << "        nop             ; Simply ignores the command" << endl;
            cout << "        abort           ; Quits execution with an error message" << endl;
            cout << "  -c, --compile         ; Compiles BF code into native binary, if possible" << endl;
            cout << "  -o X, --output=X      ; For compiling only (Default=\"a.out\")" << endl;
            cout << "  -d X, --data=X        ; Memory initialization data( ASCII file )" << endl;
            cout << "  -i, --stdin           ; Take code input from standard input instead" << endl;
            cout << "  -j, --lang=jump       ; Uses \'JumpFuck\' instead of vanilla" << endl;
            cout << "  -x [N], --lang=ext[N] ; Uses \'Extended Brainfuck Type N\' instead of vanilla" << endl;
            cout << "  --lang=X              ; Uses some other variant/extension instead of vanilla" << endl;
            cout << "        ext[1]          ; Uses \'Extended Brainfuck Type I\' instead" << endl;
            cout << "        ext2            ; Uses \'Extended Brainfuck Type II\' instead" << endl;
            cout << "        ext3            ; Uses \'Extended Brainfuck Type III\' instead" << endl;
            cout << "        jump            ; Uses \'JumpFuck\' instead" << endl;
            cout << "        love            ; Uses \'Brainlove\' instead" << endl;
            cout << "        stacked         ; Uses \'Stacked Brainfuck\' instead" << endl;
            cout << "        bcd             ; Uses \'BCDFuck\' instead" << endl;
            cout << "        stuck           ; Uses \'Brainstuck\' instead" << endl;
            cout << "        dollar          ; Uses \'Brainfuck$\' instead" << endl;
            cout << "        self-mod        ; Uses \'Self-modifying Brainfuck\' instead" << endl;
            cout << "        caret           ; Uses \'Brainfuck^\' instead" << endl;
            cout << "        bit-chan        ; Uses \'Bitchanger\' instead" << endl;
            cout << "        compressed      ; Uses \'CompressedFuck\' instead" << endl;
            cout << "        loller          ; Uses \'Brainloller\' instead" << endl;
            cout << "        ext-lol         ; Uses \'Extended Brainloller\' instead" << endl;
            cout << "        draw            ; Uses \'Drawfuck\' instead" << endl;
            exit(0);
        break;
        case 'v':
            cout << "Copyright (C) 2017, GReaperEx(Marios F.)" << endl;
            cout << "Brainfunk v" VERSION << endl;
            exit(0);
        break;
        case 'b':
            debug = true;
        break;
        case 's':
            if (!(stringstream(optarg) >> cellSize)) {
                cellSize = 0; //! Have it get handled by bsState's constructor
            }
        break;
        case 't':
            if (!(stringstream(optarg) >> cellCount)) {
                cellCount = 0;
            }
        break;
        case 'w':
            wrapPtr = true;
        break;
        case 'y':
            dynamic = true;
        break;
        case 'e':
        {
            string eofAction(optarg);
            if (eofAction == "-1") {
                onEOF = IBasicState::RETM1;
            } else if (eofAction == "0") {
                onEOF = IBasicState::RET0;
            } else if (eofAction == "nop") {
                onEOF = IBasicState::NOP;
            } else if (eofAction == "abort") {
                onEOF = IBasicState::ABORT;
            }
        }
        break;
        case 'c':
            compile = true;
        break;
        case 'o':
            if (output_file != "a.out") {
                cerr << "Warning: Output file was set more than once. Ignoring previous value." << endl;
                cerr << "       : " << output_file << " -> " << optarg << endl;
            }
            output_file = optarg;
        break;
        case 'd':
            if (dataFile != "") {
                cerr << "Warning: Data file was set more than once. Ignoring previous value." << endl;
                cerr << "       : " << dataFile << " -> " << optarg << endl;
            }
            dataFile = optarg;
        break;
        case 'i':
            useStdin = true;
        break;
        case 'j':
            useVariant = JUMP;
        break;
        case 'x':
            if (!optarg) {
                useVariant = EXTENDED;
            } else {
                switch (optarg[0])
                {
                case '1':
                    useVariant = EXTENDED;
                break;
                case '2':
                    useVariant = EXTENDED2;
                break;
                case '3':
                    useVariant = EXTENDED3;
                break;
                default:
                    cerr << "Warning: Can't understand -x type, defaulting to 1." << endl;
                    useVariant = EXTENDED;
                }
            }
        break;
        case 256:
        {
            string temp = optarg;
            if (temp == "ext" || temp == "ext1") {
                useVariant = EXTENDED;
            } else if (temp == "ext2") {
                useVariant = EXTENDED2;
            } else if (temp == "ext3") {
                useVariant = EXTENDED3;
            } else if (temp == "jump") {
                useVariant = JUMP;
            } else if (temp == "love") {
                useVariant = LOVE;
            } else if (temp == "stacked") {
                useVariant = STACKED;
            } else if (temp == "bcd") {
                useVariant = BCD;
            } else if (temp == "stuck") {
                useVariant = STUCK;
            } else if (temp == "dollar") {
                useVariant = DOLLAR;
            } else if (temp == "self-mod") {
                useVariant = SELFMOD;
            } else if (temp == "caret") {
                useVariant = CARET;
            } else if (temp == "bit-chan") {
                useVariant = BITCHAN;
            } else if (temp == "compressed") {
                useVariant = COMPRESSED;
            } else if (temp == "loller") {
                useVariant = LOLLER;
            } else if (temp == "ext-lol") {
                useVariant = EXTLOLLER;
            } else if (temp == "draw") {
                useVariant = DRAW;
            } else {
                cerr << "Warning: Can't understand requested lang, defaulting to vanilla." << endl;
                useVariant = VANILLA;
            }
        }
        break;
        case '?':
        break;
        }
    }
    for (int i = optind; i < argc; i++) {
        if (!input_file.empty()) {
            cerr << "Warning: Input file is already set, ignoring previous value." << endl;
            cerr << "       : " << input_file << " -> " << argv[i] << endl;
        }
        input_file = argv[i];
    }

    //! Running begins here

    try {
        IBasicState* myBF;

        switch (useVariant)
        {
        case VANILLA:
            myBF = new CVanillaState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case EXTENDED:
            myBF = new CExtendedState(cellSize, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case EXTENDED2:
            myBF = new CExtended2State(cellSize, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case EXTENDED3:
            myBF = new CExtended3State(cellSize, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case LOVE:
            myBF = new CLoveState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case STACKED:
            myBF = new CStackedState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case BCD:
            myBF = new CBCDState(cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
            if (cellSize != 1) {
                cerr << "Warning: Custom cell size ignored. 8-bit supported only." << endl;
            }
        break;
        case STUCK:
            myBF = new CStuckState(cellSize, cellCount, dynamic, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case JUMP:
            myBF = new CJumpState(cellSize, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case DOLLAR:
            myBF = new CDollarState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case SELFMOD:
            myBF = new CSelfmodState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case CARET:
            myBF = new CCaretState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case BITCHAN:
            myBF = new CBitchanState(cellCount, wrapPtr, dynamic, dataFile, debug);
            if (cellSize != 1) {
                cerr << "Warning: Custom cell size ignored." << endl;
            }
            if (onEOF != IBasicState::RETM1) {
                cerr << "Warning: Custom EOF policy ignored." << endl;
            }
        break;
        case COMPRESSED:
            myBF = new CCompressedState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case LOLLER:
            myBF = new CLollerState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        case EXTLOLLER:
            myBF = new CExtLollerState(cellSize, onEOF, dataFile, debug);
            if (wrapPtr) {
                cerr << "Warning: Pointer wrap-around ignored." << endl;
            }
        break;
        case DRAW:
            myBF = new CDrawState(cellSize, cellCount, wrapPtr, dynamic, onEOF, dataFile, debug);
        break;
        }

        if (useStdin) {
            if (myBF->usesBinInput()) {
                throw runtime_error("Standard input isn't a binary stream.");
            }
            myBF->translate(cin);
        } else {
            if (input_file.empty()) {
                throw runtime_error("No input file was given.");
            }
            ifstream inputStream;
            if (myBF->usesBinInput()) {
                inputStream.open(input_file, ios::binary);
            } else {
                inputStream.open(input_file);
            }
            if (!inputStream.is_open()) {
                throw runtime_error("Unable to open "+input_file+" for reading.");
            }
            myBF->translate(inputStream);
        }

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

        delete myBF;
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
