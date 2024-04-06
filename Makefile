# Variables
CC := gcc
CFLAGS := -std=c11 -Wall -Wextra -Wdouble-promotion -Werror=pedantic -Werror=vla -pedantic-errors -Wfatal-errors
DEBUG ?= 0
SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj

# Executable 
EXEC := exe

# Debug mode
ifeq ($(DEBUG), 1)
		CFLAGS += -g -O0
else
		CFLAGS += -O3
endif

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(wildcard $(INC_DIR)/*.h)

# Main target
all: $(EXEC)

# Executável
$(EXEC): $(OBJS)
		@$(CC) $(CFLAGS) -I$(INC_DIR) -o $@ $^
		@echo " Successfully compiled"

# Object files compilation
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEPS)
		@mkdir -p $(OBJ_DIR)
		$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
		@rm -rf $(OBJ_DIR) $(EXEC)
		@echo " Successfully cleaned"

rebuild: clean all

format:
		@clang-format --verbose -i $(SRC_DIR)/* $(INC_DIR)/*;
		@echo " Successfully formatted"

check-format:
		@clang-format --dry-run --Werror $(SRC_DIR)/* $(INC_DIR)/*;
		@echo " Successfully checked format"

lint:
		@clang-tidy --warnings-as-errors=* $(SRC_DIR)/* $(INC_DIR)/*;
		@echo " Successfully linted"

# Delete object files if a command fails
.DELETE_ON_ERROR:
