BUILD_DIR := bin
OBJ_DIR := bin-obj

ASSEMBLY := tests
EXTENSION :=
COMPILER_FLAGS := -g -fdeclspec -fPIC

INCLUDE_FLAGS := -Iengine/src
LINKER_FLAGS := -L./$(BUILD_DIR)/ -lengine -Wl,-rpath,.
DEFINES := -D_DEBUG -DHIMPORT

SRC_FILES := $(shell find $(ASSEMBLY) -name *.c)
DIRECTORIES := $(shell find $(ASSEMBLY) -type d)
OBJ_FILES := $(SRC_FILES:%=$(OBJ_DIR)/%.o)

all: scaffold compile link post-build

.PHONY: scaffold
scaffold:
	@echo Building $(ASSEMBLY)...
	@echo Scaffolding folder structure...
	@mkdir -p $(addprefix $(OBJ_DIR)/.$(DIRECTORIES))
	@echo Done.

.PHONY: link
link: scaffold $(OBJ_FILES)
	@echo Linking $(ASSEMBLY)...
	@clang $(OBJ_FILES) -o $(BUILD_DIR)/$(ASSEMBLY)$(EXTENSION) $(LINKER_FLAGS)

.PHONY: compile
compile:
	@echo Compiling...

.PHONY: post-build
post-build:
	@echo Build Finished.

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/$(ASSEMBLY)
	rm -rf $(OBJ_DIR)/$(ASSEMBLY)

$(OBJ_DIR)/%.c.o: %.c
	@echo	$<...
	@clang $< $(COMPILER_FLAGS) -c -o $@ $(DEFINES) $(INCLUDE_FLAGS)