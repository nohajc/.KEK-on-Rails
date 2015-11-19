#!/bin/bash

# run from git root dir
# you can set args for vm ie: ./tests/run.sh -d all
adir="$(pwd)"
compilerdir="$adir/./compiler"
vmdir="$adir/./vm"
testsdir="$adir/./tests"

set -exv

make -C compiler
valgrind --leak-check=full --track-origins=yes -q  $compilerdir/main $testsdir/hello_world.ke{k,xe}

make -C vm
valgrind --leak-check=full --track-origins=yes -q  $vmdir/kek $@ $testsdir/hello_world.kexe

set +exv

