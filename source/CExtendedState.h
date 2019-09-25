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

#ifndef CEXTENDED_STATE_H
#define CEXTENDED_STATE_H

#include "CVanillaState.h"

class CExtendedState : public CVanillaState
{
public:
    CExtendedState(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CExtendedState();

    //! Converts BF code to manageable token blocks, compressed/optimized if possible
    void translate(std::istream& input);

protected:
    CellType storage; //! Extra storage cell required by Extended Type I

    void compilePreInst(std::ostream& output);
    void runInstruction(const BFinstr& instr);
    void compileInstruction(std::ostream& output, const BFinstr& instr);

    void runDebug();

    virtual const CellType getStorage() const {
        return storage;
    }
};

#endif // CEXTENDED_STATE_H
