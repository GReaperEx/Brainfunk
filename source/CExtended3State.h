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

#ifndef CEXTENDED3_STATE_H
#define CEXTENDED3_STATE_H

#include "CExtended2State.h"

#include <map>

class CExtended3State : public CExtended2State
{
public:
    CExtended3State(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CExtended3State();

    //! Prepares memory and code for execution
    void translate(std::istream& input);

    //! Just throws an error
    void compile(std::ostream& output);

protected:
    int initPtrPos; //! Because of 'x' command, it needs to remember this

    std::map<int, bool> lockMap; //! It also allows for setting read-only memory
    std::vector<int> prevPtrs; //! Stack for saving curPtrPos, for use with 'X' command

    bool isLocked(int cellIndex) {
        auto iter = lockMap.find(cellIndex);
        if (iter != lockMap.end()) {
            return iter->second;
        }
        return false;
    }

    void runInstruction(const BFinstr& instr);
};

#endif // CEXTENDED3_STATE_H
