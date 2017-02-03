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

#ifndef CVANILLA_STATE_H
#define CVANILLA_STATE_H

#include "IBasicState.h"

class CVanillaState : public IBasicState
{
public:
    /**
        size        : The size of each cell, acceptable values are 1, 2, 4 or 8
        count       : The amount of cells available, meaningless if dynamicTape == true
        wrapPtr     : Wraps the tape pointer around, can't be true if dynamicTape is also true
        dynamicTape : Makes the available tape grow dynamically when accessing out of upper bounds
     */
    CVanillaState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CVanillaState();

    //! Converts BF code to manageable token blocks, compressed/optimized if possible
    void translate(std::istream& input);
    //! Runs translated code
    void run();
    //! Compiles translated code into C source
    void compile(std::ostream& output);

protected:
    void* tape;

    int cellSize;
    int cellCount;

    bool ptrWrap;
    bool dynamic;

    ActionOnEOF eofPolicy;

    int curPtrPos; //! Selected memory cell
    unsigned IP;   //! Interpretor only, pseudo Instruction Pointer

    bool keepRunning;

    struct BFinstr
    {
        char token;
        int repeat;

        BFinstr(char t): token(t), repeat(1) {}
        void incr() { ++repeat; }
    };
    std::vector<BFinstr> instructions;

    std::vector<CellType> initData;

    const CellType getCell(int cellIndex);
    void setCell(int cellIndex, const CellType& newValue);

    //! Understands escape sequences, symbol, octal and hex
    //! Only hex values can be > 255( technically, octal too but not for much )
    void parseData(std::istream& input);

    bool userInput(uint8_t& c);

    void examineIndex(int& cellIndex);

    virtual void compilePreMain(std::ostream& output);
    virtual void compilePreInst(std::ostream& output);
    virtual void compileCleanup(std::ostream& output);

    virtual void runInstruction(const BFinstr& instr);
    virtual void compileInstruction(std::ostream& output, const BFinstr& instr);

    virtual BFinstr& getCode(int ip);

    virtual bool hasInstructions() const {
        return !instructions.empty();
    }

    virtual void runDebug();

    bool doDebug; //! Flag to enable or disable debugging
    volatile bool dbgPaused;

    static void signalHandle(int) {
        if (!CVanillaState::state->dbgPaused) {
            CVanillaState::state->dbgPaused = true;
        } else {
            exit(-1);
        }
    }
    static CVanillaState* state;
};

#endif // CVANILLA_STATE_H
