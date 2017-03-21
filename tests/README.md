# Creating tests for Brainfunk

Simple testing system, but hey, it works.

Each testcase should have its own directory, everything looks cleaner that way.

## Required files
| Extension | Usefulness |
| --- | --- |
| *.bf  | Contains the Brainfuck code you want to run and test |
| *.c.bf  | Contains the Brainfuck code you want to compile and test |
| *.in  | Any desired input should go here |
| *.val | Contains valid/ideal output. Beware of extra newlines! |
| *.use | All the cmd-line arguments it should pass to bfk |

## Generated files
| Extension | Usefulness |
| --- | --- |
| *.out | Contains the actual output of a script |
| *.exe | Generated executable to test compilation |

If you find that your editor inserts a newline at the end of every file, you may want to correct it **at least** for *.val files.
Remember to never *.use '-c' or '--compile' because that would break the automatic testing.

