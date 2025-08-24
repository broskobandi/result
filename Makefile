# Project
PROJECT := result
CC := $(shell command -v clang || command -v gcc)
CFLAGS := -Wall -Wextra -Werror -Wconversion -Wunused-result
CPPFLAGS := -Isrc -Iinclude

# Dirs
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
TEST_OBJ_DIR := $(BUILD_DIR)/test-obj
TEST_DIR := test
SRC_DIR := src
INC_DIR := include
LIB_INSTALL_DIR := /usr/local/lib
INC_INSTALL_DIR := /usr/local/include
DOC_DIR := doc

# Files
INC := $(INC_DIR)/$(PROJECT).h
SRC = $(wildcard $(SRC_DIR)/*.c)
INC_PRIV := $(wildcard $(SRC_DIR)/*.h)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TEST_MAIN := $(TEST_DIR)/main/test.c
TEST_INC_PRIV := $(wildcard $(TEST_DIR)/*.h)
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_EXE := $(BUILD_DIR)/test
TEST_OBJ := $(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_OBJ_DIR)/%.o)
LIB_A := $(BUILD_DIR)/lib$(PROJECT).a
LIB_SO := $(BUILD_DIR)/lib$(PROJECT).so

# Rules:
.PHONY: all test clean install uninstall doc

all: $(LIB_A) $(LIB_SO)

$(LIB_A): $(OBJ) | $(BUILD_DIR)
	ar rcs $@ $^

$(LIB_SO): $(OBJ) | $(BUILD_DIR)
	$(CC) -shared $(CFLAGS) $(CPPFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INC_PRIV) $(INC) | $(OBJ_DIR)
	bear -- $(CC) -c -fPIC $(CFLAGS) $(CPPFLAGS) $< -o $@

$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c $(TEST_INC_PRIV) $(INC_PRIV) $(INC) | $(TEST_OBJ_DIR)
	bear -- $(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(TEST_EXE): $(TEST_MAIN) $(TEST_OBJ) $(OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $^ -o $@

$(BUILD_DIR):
	mkdir -p $@

$(OBJ_DIR):
	mkdir -p $@

$(TEST_OBJ_DIR):
	mkdir -p $@

test: CPPFLAGS += -DTEST
test: $(TEST_EXE)
	./$<

doc: $(INC) $(INC_PRIV) $(SRC)
	doxygen

clean:
	rm -rf $(BUILD_DIR) compile_commands.json $(DOC_DIR)

install:
	cp $(LIB_SO) $(LIB_INSTALL_DIR)/
	cp $(LIB_A) $(LIB_INSTALL_DIR)/
	cp $(INC) $(INC_INSTALL_DIR)/

uninstall:
	rm -rf $(addprefix $(LIB_INSTALL_DIR)/, $(notdir $(LIB_SO)))
	rm -rf $(addprefix $(LIB_INSTALL_DIR)/, $(notdir $(LIB_A)))
	rm -rf $(addprefix $(INC_INSTALL_DIR)/, $(notdir $(INC)))
