BUILD_DIR := bin
OBJ_DIR := bin-obj

ASSEMBLY := engine
EXTENSION := .so
COMPILER_FLAGS := -g -fdeclspec -fPIC

INCLUDE_FLAGS := -Iengine/src
LINKER_FLAGS := -g -shared -lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/X11R6/lib -lGL -lGLEW -lm
DEFINES := -D_DEBUG -DHEXPORT

SRC_FILES := $(shell find $(ASSEMBLY) -name *.c)
DIRECTORIES := $(shell find $(ASSEMBLY) -type d)
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)

all: scaffold compile link post-build

.PHONY: scaffold
scaffold:
	@echo Building $(ASSEMBLY)...
	@echo Scaffolding folder structure...
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(addprefix $(OBJ_DIR)/,$(DIRECTORIES))
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES)
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/lib$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile:
	@echo Compiling...

.PHONY: post-build
post-build:
	@echo Copying shaders...
	@mkdir -p bin/assets
	@cp -R assets bin/assets
	@echo Build Finished.

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/$(ASSEMBLY)
	rm -rf $(OBJ_DIR)/$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)