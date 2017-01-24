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

#ifndef CEXTENDED3_STATE_H
#define CEXTENDED3_STATE_H

#include "IBasicState.h"

#include <map>
#include <vector>

class CExtended3State : public IBasicState
{
public:
    CExtended3State(int size, const std::string& dataFile);
    ~CExtended3State();

    //! Prepares memory and code for execution
    void translate(std::istream& input);
    //! Runs translated code
    void run();
    //! Just throws an error
    void compile(std::ostream& output);

private:
    int initPtrPos; //! Because of 'x' command, it needs to remember this
    int curPtrPos; //! Selected memory cell
    int IP;   //! Actual Instruction Pointer

    int storagePos; //! Type III makes the storage bit relocatable
    std::map<int, bool> lockMap; //! It also allows for setting read-only memory

    std::vector<int> prevPtrs; //! Stack for saving curPtrPos, for use with 'X' command

    bool isLocked(int cellIndex) {
        auto iter = lockMap.find(cellIndex);
        if (iter != lockMap.end()) {
            return iter->second;
        }
        return false;
    }
};

#endif // CEXTENDED3_STATE_H
