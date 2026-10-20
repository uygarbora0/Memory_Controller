# Source files
C_SRCS := src/main.c src/csv_parser.c src/field_parsers.c src/parser_utilities.c src/rom_parser.c
CPP_SRCS := src/memory_controller.cpp

# Object files (build directory)
OBJ_DIR := build
C_OBJS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(C_SRCS))
CPP_OBJS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(CPP_SRCS))

# Target name
TARGET := project

# SystemC installation path (set this before running make, e.g. export SYSTEMC_HOME=...)
SCPATH := $(SYSTEMC_HOME)

# Compiler commands
CXX := $(shell command -v g++ || command -v clang++)
CC := $(shell command -v gcc || command -v clang)

# Compiler flags
CFLAGS := -Wall -Wextra -O2 -Iinclude -I$(SCPATH)/include
CXXFLAGS := -Wall -Wextra -std=c++14 -O2 -Iinclude -I$(SCPATH)/include

# Linker flags
LDFLAGS = -L$(SYSTEMC_HOME)/lib -lsystemc -lm -Wl,-rpath,$(SYSTEMC_HOME)/lib

# Add rpath for non-Mac OS systems
UNAME_S := $(shell uname -s)
ifneq ($(UNAME_S), Darwin)
    LDFLAGS += -Wl,-rpath=$(SCPATH)/lib
endif

# ---------------------------------------
# CONFIGURATION END
# ---------------------------------------

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(C_OBJS) $(CPP_OBJS) $(LDFLAGS) -o $@

# Compile C source files to object files
$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source files to object files
$(OBJ_DIR)/%.o: src/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean