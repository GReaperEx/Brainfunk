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

#ifndef IBASIC_STATE_H
#define IBASIC_STATE_H

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <sstream>
#include <iomanip>
#include <vector>

class IBasicState
{
public:
    enum ActionOnEOF { RETM1, RET0, NOP, ABORT };

    virtual ~IBasicState() {}

    virtual void translate(std::istream& input) = 0;
    virtual void run() = 0;
    virtual void compile(std::ostream& output) = 0;

    virtual bool usesBinInput() const {
        return false;
    }

protected:
    union CellType
    {
        uint8_t  c8;
        uint16_t c16;
        uint32_t c32;
        uint64_t c64;

        bool operator< (const CellType& other) const{
            return c64 < other.c64;
        }
    };
};

#endif // IBASIC_STATE_H
