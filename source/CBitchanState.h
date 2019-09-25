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

#ifndef CBITCHAN_STATE_H
#define CBITCHAN_STATE_H

#include "CVanillaState.h"

class CBitchanState : public CVanillaState
{
public:
    CBitchanState(int count, bool wrapPtr, bool dynamicTape, const std::string& dataFile, bool debug);
    ~CBitchanState();

    void translate(std::istream& input);

protected:
    const CellType getCell(int cellIndex);
    void setCell(int cellIndex, const CellType& newValue);

    void compilePreInst(std::ostream& output);

    void runInstruction(const BFinstr& instr);
    void compileInstruction(std::ostream& output, const BFinstr& instr);
};

#endif // CBITCHAN_STATE_H
