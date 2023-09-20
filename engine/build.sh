#!/bin/bash

set echo on

mkdir -p ../bin

cFilenames=$(find . -type f -name "*.c")

assembly="engine"
compilerFlags="-g -shared -fdeclspec -fPIC"

glewInclude="vendor/glew-2.2.0/include/"
glewLib="vendor/glew-2.2.0/lib/Release/x64/glew32s"

includeFlags="-Isrc -I$glewInclude"
linkerFlags="-lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/X11R6/lib -lGL -l$glewLib"
defines="-D_DEBUG -DHEXPORT"

echo "Building $assembly..."
echo clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags
clang $cFilenames $compilerFlags -o ../bin/lib$assembly.so $defines $includeFlags $linkerFlags