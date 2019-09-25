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

#ifndef CBCD_STATE_H
#define CBCD_STATE_H

#include "CVanillaState.h"

//! I didn't change this class because I didn't want to mess with it yet( it's a bit complex )

class CBCDState : public CVanillaState
{
public:
    CBCDState(int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CBCDState();

    //! Converts BF code to manageable token blocks, compressed/optimized if possible
    void translate(std::istream& input);
    //! Runs translated code
    void run();
    //! Compiles translated code into C source
    void compile(std::ostream& output);

private:
    int curPtrPos; //! Selected memory cell
    unsigned IP;   //! Instruction Pointer

    bool swapFunctions; //! Feature of BCDFuck, swaps the tape with the code

    std::vector<uint8_t> instructions;

    uint8_t _getCell(int cellIndex);
    void _setCell(int cellIndex, uint8_t newVal);

    uint8_t _getCode(int codeIndex);
    void _setCode(int codeIndex, uint8_t newVal);

    size_t _getCodeSize();

    uint8_t bufInput;
    bool isInputBuf;
    uint8_t bufOutput;
    bool isOutputBuf;

    void getInput(uint8_t& c) {
        if (!isInputBuf) {
            userInput(bufInput);
        }

        if (!isInputBuf) {
            isInputBuf = true;
            c = bufInput >> 4;
        } else {
            isInputBuf = false;
            c = bufInput & 0xF;
        }
    }

    void showOutput(uint8_t nibble) {
        if (!isOutputBuf) {
            isOutputBuf = true;
            bufOutput |= nibble << 4;
        } else {
            isOutputBuf = false;
            bufOutput |= nibble;

            std::cout.put(bufOutput);
            bufOutput = 0;
        }
    }

    bool hasInstructions() const {
        return !instructions.empty();
    }
};

#endif // CBCD_STATE_H
