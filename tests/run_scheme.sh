make -C ../compiler && ../compiler/kekc scheme.kek ../tests/kexes/scheme.kexe && make -C ../vm && valgrind --show-leak-kinds=all --leak-check=full --track-origins=yes ../vm/kek ../tests/kexes/scheme.kexe
