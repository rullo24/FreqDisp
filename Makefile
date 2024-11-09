# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror # compiled dynamically for Mesa OpenGL implementation

# Raylib and additional libraries
RAYLIB_LIB_DIR = ../00-dependencies/raylib/src/
RAYLIB_INC_DIR = ../00-dependencies/raylib/raylib/include
RAYGUI_INC_DIR = ../00-dependencies/raygui/src

INCS = -I$(RAYLIB_INC_DIR) -I$(RAYGUI_INC_DIR)
LIBS = -L$(RAYLIB_LIB_DIR) -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

# Source and output files
SRC = main.c  # Replace with your source files if more than one, e.g., main.c utils.c
OUT = freqdisp  # The name of the output executable

# Default target to compile the program
all: $(OUT)
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(INCS) $(LIBS)

# Clean target to remove the compiled output
clean:
	rm $(OUT)
