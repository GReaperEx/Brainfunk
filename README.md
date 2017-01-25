# Brainfunk

Brainfuck optimizing interpreter and compiler. Aims to support all extensions/variations of the language.

Available features up-to-date:
* Variable cell size( each can be 1, 2, 4 or 8 bytes long )
* Variable cell amount( anything from 1 to over 2 billion )
* Option to wrap the pointer between bounds
* Option to have dynamic tape length( ignores pointer wrap )
* Option to compile instead of run, if possible
* Optional memory initialization, data file

Supported languages up-to-date:
* Brainfuck
* Extended Brainfuck
  * Type I
  * Type II
  * Type III
* Brainlove
* Stacked Brainfuck
* BCDFuck

