#!/bin/bash

set echo on

mkdir -p ../bin

cFilenames=$(find . -type f -name "*.c")

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"

includeFlags="-Isrc"
linkerFlags="-lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/X11R6/lib"
defines="-D_DEBUG -DHEXPORT"

echo "Building $assembly..."
echo clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags