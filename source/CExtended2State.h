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

#include "IBasicState.h"

class CExtended2State : public IBasicState
{
public:
    CExtended2State(int size, const std::string& dataFile);
    ~CExtended2State();

    //! Prepares memory and code for execution
    void translate(std::istream& input);
    //! Runs translated code
    void run();
    //! Just throws an error
    void compile(std::ostream& output);

private:
    int curPtrPos; //! Selected memory cell
    int IP;   //! Actual Instruction Pointer
};

#endif // CEXTENDED2_STATE_H
