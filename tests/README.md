#Creating unit-tests for Brainfunk

Simple testing system, but hey, it works. 

##Required files
| *.bf  | Contains the Brainfuck code you want to test |
| *.in  | Any desired input should go here |
| *.val | Contains valid/ideal output. Beware of extra newlines! |
| *.use | All the cmd-line arguments it should pass to bfk |

##Generated files:
| *.out    | Contains the actual output of a script |
| ../a.out | Temporary executable to test compilation |

If you find that your editor inserts a newline at the end of every file, you may want to correct it **at least** for *.val files.
Remember to never *.use '-c' or '--compile' because that would break the automatic testing.

