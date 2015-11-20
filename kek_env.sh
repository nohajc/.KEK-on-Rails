#!/bin/bash

# cd to script directory
cd ${0%/*}

# run shell with modified environment
bash --rcfile <(cat ~/.bashrc; echo "PATH=$PWD/vm:$PWD/compiler:\$PATH PS1=(kek)\$PS1")
