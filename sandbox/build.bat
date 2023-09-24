REM Build script for sandbox
@ECHO off
SetLocal EnableDelayedExpansion

SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! %%f
)

SET assembly=sandbox
SET compilerFlags=-g
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../bin/ -lengine.lib
SET defines=-D_DEBUG -DHIMPORT

ECHO "Building %assembly%..."
REM ECHO clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.exe %defines% %includeFlags% %linkerFlags%