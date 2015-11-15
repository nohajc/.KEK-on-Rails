#!/bin/bash

adir="$(pwd)"
compilerdir="$adir/./compiler"
vmdir="$adir/./vm"
testsdir="$adir/./tests"

set -exv

make -C compiler
valgrind --leak-check=full --track-origins=yes -q  $compilerdir/main $testsdir/hello_world.ke{k,xe}

make -C vm
valgrind --leak-check=full --track-origins=yes -q  $vmdir/kek -f $testsdir/hello_world.kexe

set +exv

