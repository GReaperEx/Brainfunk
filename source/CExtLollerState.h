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

#ifndef CEXT_LOLLER_STATE_H
#define CEXT_LOLLER_STATE_H

#include "CExtendedState.h"

class CExtLollerState : public CExtendedState
{
public:
    CExtLollerState(int size, ActionOnEOF onEOF, const std::string& dataFile, bool debug);
    ~CExtLollerState();

    void translate(std::istream& input);

    bool usesBinInput() const {
        return true;
    }
};

#endif // CEXT_LOLLER_STATE_H
