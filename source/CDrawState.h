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

#ifndef CDRAW_STATE_H
#define CDRAW_STATE_H

#include "CVanillaState.h"
#include <png++/png.hpp>

class CDrawState : public CVanillaState
{
public:
    CDrawState(int size, int count, bool wrapPtr, bool dynamicTape, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CDrawState();

    void translate(std::istream& input);

    void compile(std::ostream& output); //! Cannot compile Drawfuck, at least for now

protected:
    uint8_t R, G, B;
    uint8_t X, Y;
    png::image<png::rgb_pixel> outimg;

    void runInstruction(const BFinstr& instr);
    void runDebug();
};

#endif // CDRAW_STATE_H

