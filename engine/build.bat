REM Build script for engine
@ECHO off
SetLocal EnableDelayedExpansion

SET cFilenames=
FOR /R %%f in (*.c) do (
    SET cFilenames=!cFilenames! %%f
)

SET glewInclude=vendor/glew-2.2.0/include/
SET glewLib=vendor/glew-2.2.0/lib/Release/x64/glew32s

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror

SET includeFlags=-Isrc -I%glewInclude%
SET linkerFlags=-luser32 -lOpengl32 -lGdi32 -l%glewLib%
SET defines=-D_DEBUG -DHEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
REM ECHO clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%
clang %cFilenames% %compilerFlags% -o ../bin/%assembly%.dll %defines% %includeFlags% %linkerFlags%