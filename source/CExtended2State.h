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

#ifndef CEXTENDED2_STATE_H
#define CEXTENDED2_STATE_H

#include "CVanillaState.h"

class CExtended2State : public CVanillaState
{
public:
    CExtended2State(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CExtended2State();

    //! Prepares memory and code for execution
    void translate(std::istream& input);

    //! Just throws an error
    void compile(std::ostream& output);

protected:
    int storagePos;

    void runInstruction(const BFinstr& instr);
    BFinstr& getCode(int ip);

    bool hasInstructions() const {
        return true;
    }

    const CellType getStorage() {
        return getCell(storagePos);
    }
};

#endif // CEXTENDED2_STATE_H
