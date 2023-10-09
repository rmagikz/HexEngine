DIR := $(subst /,\,${CURDIR})
BUILD_DIR := bin
OBJ_DIR := bin-obj

ASSEMBLY := engine
EXTENSION := .dll
COMPILER_FLAGS := -g -fdeclspec

GLEW := engine\vendor\glew-2.2.0
STB := engine\vendor\stb_image

INCLUDE_FLAGS := -Iengine\src -I$(GLEW)\include -I$(STB)
LINKER_FLAGS := -g -shared -luser32  -lOpengl32 -lGdi32 -l$(GLEW)\lib\Release\x64\glew32s
DEFINES := -D_DEBUG -DHEXPORT -D_CRT_SECURE_NO_WARNINGS

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

SRC_FILES := $(call rwildcard,$(ASSEMBLY)/,*.c)
DIRECTORIES := \$(ASSEMBLY)\src $(subst $(DIR),,$(shell dir $(ASSEMBLY)\src /S/ AD/ B | findstr /i src))
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)

all: scaffold compile link
.PHONY: scaffold
scaffold:
	@echo Building $(ASSEMBLY)...
	@echo Scaffolding folder structure...
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(addprefix $(OBJ_DIR), $(DIRECTORIES)) 2>NUL || cd .
	-@setlocal enableextensions enabledelayedexpansion && mkdir $(BUILD_DIR) 2>NUL || cd .
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES)
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile:
	@echo Compiling...

.PHONY: clean
clean:
	if exist $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION) del $(BUILD_DIR)\$(ASSEMBLY)$(EXTENSION)
	rmdir /s /q $(OBJ_DIR)\$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)