gcc foamasm.c -o foamasm
gcc foamdasm.c -o foamdasm
gcc foamvm.c cli.c -o foamcli
gcc foamvm.c sdl.c -o foamsdl -lSDL2
