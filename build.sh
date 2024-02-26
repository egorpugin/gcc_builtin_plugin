#!/bin/bash

set -e

COMPILER=g++
#COMPILER=../build/install/bin/g++
FILE=plugin

#PATH=$PATH:../build/gcc

$COMPILER -I`$COMPILER -print-file-name=plugin`/include -DSYSTEM_BUILD -fPIC -shared -fno-rtti -g3 -O0 $FILE.cpp -o plugin.so
$COMPILER -std=c++2b -fplugin=./plugin.so test.cpp -g3 -O0 -fdump-tree-all # -wrapper gdb,--args
./a.out && echo $? || echo $?
#$COMPILER -std=c++2b -fplugin=./plugin.so test.cpp     -O3 -fdump-tree-all # -wrapper gdb,--args
#./a.out && echo $? || echo $?

