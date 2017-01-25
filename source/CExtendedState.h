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

#ifndef CEXTENDED_STATE_H
#define CEXTENDED_STATE_H

#include "IBasicState.h"

class CExtendedState : public IBasicState
{
public:
    CExtendedState(int size, const std::string& dataFile);
    ~CExtendedState();

    //! Converts BF code to manageable token blocks, compressed/optimized if possible
    void translate(std::istream& input);
    //! Runs translated code
    void run();
    //! Compiles translated code into C source
    void compile(std::ostream& output);

private:
    int curPtrPos; //! Selected memory cell
    unsigned IP;   //! Interpretor only, pseudo Instruction Pointer

    CellType storage; //! Extra storage cell required by Extended Type I

    struct BFinstr
    {
        char token;
        int repeat;

        BFinstr(char t): token(t), repeat(1) {}
        void incr() { ++repeat; }
    };
    std::vector<BFinstr> instructions;
};

#endif // CEXTENDED_STATE_H
