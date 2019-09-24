# Brainfunk

Brainfuck optimizing interpreter and compiler. Aims to support all extensions/variations of the language.

Available features up-to-date:
* Debug mode with stepping and tape manipulation
* Variable cell size( each can be 1, 2, 4 or 8 bytes long )
* Variable cell amount( anything from 1 to over 2 billion )
* Option to wrap the pointer between bounds
* Option to have dynamic tape length( ignores pointer wrap )
* Option to compile instead of run, if possible
* Optional memory initialization, data file
* Action on EOF is configurable( -1, 0, nop or abort )
* Option to read code from standard input

Supported languages up-to-date:
* Brainfuck
* Extended Brainfuck
  * Type I
  * Type II
  * Type III
* BCDFuck
* Bitchanger
* Brainfuck^
* Brainfuck$
* Brainloller
* Extended Brainloller
* Brainlove
* Brainstuck
* CompressedFuck
* JumpFuck
* Self-modifying Brainfuck
* Stacked Brainfuck
* Drawfuck

Libraries and programs required for compilation:
* png++ 0.2.5 or later( and as an extension, libpng 1.2.x )
* g++( any with decent C++11 support )
* GNU Make( any decently recent version )

Programs required for running:
* gcc( any decently recent version )


Copyright (C) 2017-2019, GReaperEx(Marios F.)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 only.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
