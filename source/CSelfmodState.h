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

#ifndef CSELFMOD_STATE_H
#define CSELFMOD_STATE_H

#include "CVanillaState.h"

class CSelfmodState : public CVanillaState
{
public:
    CSelfmodState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CSelfmodState();

    //! Prepares code to be run
    void translate(std::istream& input);

    //! Does nothing here
    void compile(std::ostream& output);

protected:
    void runInstruction(const BFinstr& instr);
    BFinstr& getCode(int ip);

    bool hasInstructions() const {
        return true;
    }
};

#endif // CSELFMOD_STATE_H
