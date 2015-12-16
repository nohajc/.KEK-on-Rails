# .KEK-on-Rails
A C-like language interpreter
<pre>
          _  __    ___    _  __  
         | |/ /   | __|  | |/ /  
    _    | ' <    | _|   | ' <   
  _(_)_  |_|\_\   |___|  |_|\_\  
_|"""""|_|"""""|_|"""""|_|"""""| 
"`-0-0-'"`-0-0-'"`-0-0-'"`-0-0-' 
</pre>

# FAQ

- Q: i've clonned .kek but i can't find it plshelp
- A: git clone https://github.com/nohajc/.KEK-on-Rails dotKEK-on-Rails

# Features
- Object oriented language with inheritance and dynamic dispatch
- Dynamic type system
- Exception handling
- Growing arrays (like C++ vectors)
- Tail call optimization (automatically detects RET after CALL and reuses stack frame)
- Tagged integers and chars
- Inline cache
- Cheney's copying GC

# Garbage collector flags
If you're testing something and it doesn't work, it's a good idea to turn off gc completely. Because there may be a bug in gc. There is a list of available options:
 - **-gN** or **-g none** no gc at all. The virtual machine will use just malloc underneath. All memory is stored in a global LL and free'd at the end of the vm. state: **working without any known bugs**
 - **-gn** or **-g new** only new space. All objects are created in to-space. When there is no space for an obj we want to allocate, the Cheney's copying GC swaps to- and from- space, and copy live object back to the to- space. state: **working without any known bugs**
 - **-gg** or **-g gen** generational gc. It's almost the same as the previous mehthod, but if an obj survives in new space for two Cheney's copying, the object is moved into the old space. state: **there are some rare bugs**
 - **-gG** or **-g genmas** generational gc with mark-and-sweep algorithm in old space. state: **not yet implemented**
 - **-gM** or **-g genmasmac** generational gc with mark-and-sweep and mark-and-compact algorithm in old space. state: **not yet implemented**

# TODO
- generational gc with mark-and-sweep and mark-and-copy
- one classfile per class
- native class wrapping SDL
- lambda functions
- closures
- continuations
- JIT

# Debugging
Life is hard, but it's even harder when you program in C. Programming in reality is really different than what we can see in movies. We used Valgrind and GDB.

## Valgrind
Example of running scheme from the root directory of the repository:

          make -C ../compiler && ../compiler/kekc scheme.kek ../tests/kexes/scheme.kexe && make -C ../vm && valgrind --show-leak-kinds=all --leak-check=full --track-origins=yes ../vm/kek ../tests/kexes/scheme.kexe

## GDB
If you're lazy to write GDB command over and over, here are some one-liners;

          make -C ../vm && gdb -ex "set confirm off" -ex "file ../vm/kek" [INSERT BREAKPOINT HERE] -ex "set args ../tests/kexes/scheme.kexe sat.scm sat_in.txt" -ex "r"

some breakpoints we've used:
 - -ex "break vm.c:616 if tick == 10450"
 - -ex "watch *0x7ffff6da51d0"


