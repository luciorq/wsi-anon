#
# Makefile to run under Windows
#

# directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# final target
CONSOLE_TARGET := console-app

# list files
SOURCEFILES = $(filter-out $(SRC_DIR)/js-file.c $(SRC_DIR)/wsi-anonymizer-wasm.c, $(wildcard $(SRC_DIR)/*.c))
OBJECTFILES := $(SOURCEFILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# flags
GCC = gcc
C_FLAG = -c
O_FLAG = -o
F_FLAG = -f
R_FLAG = -r
L_FLAGS   = -Wall -I.
RM_DIR = rd /s /q
RM = rm

$(CONSOLE_TARGET): $(BIN_DIR)/$(CONSOLE_TARGET)

$(BIN_DIR)/$(CONSOLE_TARGET): makedirs $(OBJECTFILES)
	@$(GCC) $(OBJECTFILES) $(L_FLAGS) $(O_FLAG) $@
	@echo Linking complete!

$(OBJECTFILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	@$(GCC) $(C_FLAG) $< $(O_FLAG) $@
	@echo Compiling $< ...

makedirs: 
	@if exist $(OBJ_DIR) $(RM_DIR) $(OBJ_DIR)
	@if exist $(BIN_DIR) $(RM_DIR) $(BIN_DIR)
	@mkdir $(OBJ_DIR)
	@mkdir $(BIN_DIR)

.PHONY: clean

clean:
	@$(RM_DIR) $(OBJ_DIR)
	@$(RM_DIR) $(BIN_DIR)
	@echo Cleanup complete!

# Done:
# - corrected tabulator
# - support anonymization of mirax, aperio
# - force overwriting on windows
# - correct makefile for windows