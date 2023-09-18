REM Build script for engine
@ECHO off
SetLocal EnableDelayedExpansion

SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! %%f
)

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
SET includeFlags=-Isrc
SET linkerFlags=-luser32
SET defines=-D_DEBUG -DHEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%