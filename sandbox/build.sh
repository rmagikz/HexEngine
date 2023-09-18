#!/bin/bash

set echo on

mkdir -p ../bin

cFilenames=$(find . -type f -name ".c")

assembly="sandbox"
compilerFlags="-g -fdeclspec -fPIC"

includeFlags="-Isrc -I../engine/src/"
linkerFlags="-L../bin/ -lengine -Wl,-rpath,."
defines="-D_DEBUG -DHIMPORT"

echo "Building $assembly..."
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags