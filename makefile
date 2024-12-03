# Compiler to use
C ?= gcc

# src directory path
SRC_PATH = src

# Extension of source files to compile
INCLUDE_EXT = c

# Output file 
OUTPUT = thoth.exe

all: 
	$(C) -O2 $(SRC_PATH)/*.$(INCLUDE_EXT) -o $(OUTPUT)