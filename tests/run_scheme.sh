#valgrind:

make -C ../compiler && ../compiler/kekc scheme.kek ../tests/kexes/scheme.kexe && make -C ../vm && valgrind --show-leak-kinds=all --leak-check=full --track-origins=yes ../vm/kek ../tests/kexes/scheme.kexe sat.scm sat_in.txt

#gdb:

make -C ../vm && gdb -ex "set confirm off" -ex "file ../vm/kek" [INSERT BP HERE]  -ex "set args ../tests/kexes/scheme.kexe sat.scm sat_in.txt" -ex "r"

# gdb bp examples:
-ex "break vm.c:616 if tick == 10450"
-ex "watch *0x7ffff6da51d0"

make -C ../vm && gdb -ex "set confirm off" -ex "file ../vm/kek" -ex "watch *0x7ffff6da51d0" -ex "set args ../tests/kexes/scheme.kexe sat.scm sat_in.txt" -ex "r"
