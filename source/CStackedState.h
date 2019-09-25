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

#ifndef CSTACKED_STATE_H
#define CSTACKED_STATE_H

#include "CVanillaState.h"

class CStackedState : public CVanillaState
{
public:
    CStackedState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CStackedState();

    //! Converts BF code to manageable token blocks, compressed/optimized if possible
    void translate(std::istream& input);

protected:
    std::vector<CellType> cellStack;

    void compilePreMain(std::ostream& output);
    void compilePreInst(std::ostream& output);
    void compileCleanup(std::ostream& output);

    void runInstruction(const BFinstr& instr);
    void compileInstruction(std::ostream& output, const BFinstr& instr);

    void runDebug();
};

#endif // CSTACKED_STATE_H
