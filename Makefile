PLATFORM := unknown

BUILD_DIR := bin
OBJ_DIR := bin-obj

ifeq ($(OS), Windows_NT)
	PLATFORM := windows
else
	PLATFORM := linux
endif

ifeq ($(PLATFORM), unknown)
	@echo Platform unknown
	@exit
endif

.PHONY: all
all:
	@echo Detected platform: $(PLATFORM)
	@echo Building all...
	@make -f "engine/Makefile.engine.$(PLATFORM).mak" all
	@make -f "sandbox/Makefile.sandbox.$(PLATFORM).mak" all
	@make -f "tests/Makefile.tests.$(PLATFORM).mak" all

.PHONY: clean
clean:
	@echo Cleaning...
	@make -f "sandbox/Makefile.sandbox.$(PLATFORM).mak" clean
	@make -f "tests/Makefile.tests.$(PLATFORM).mak" clean
	@make -f "engine/Makefile.engine.$(PLATFORM).mak" clean