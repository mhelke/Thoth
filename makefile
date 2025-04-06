# Compiler to use
C ?= gcc

PROFILER = gprof
PROFILE_OUTPUT = profile.txt
PROFILE = -pg -no-pie

DEBUG_ARG = debug

# src directory path
SRC_PATH = src

# Extension of source files to compile
INCLUDE_EXT = c

# Output file 
OUTPUT = thoth.exe
DEBUG_OUTPUT = thoth-debug.exe

all: 
	$(C) -O2 $(SRC_PATH)/*.$(INCLUDE_EXT) -o $(OUTPUT)

debug:
	$(C) $(SRC_PATH)/*.$(INCLUDE_EXT) -o $(DEBUG_OUTPUT)

profile:
	$(C) -O2 $(PROFILE) $(SRC_PATH)/*.$(INCLUDE_EXT) -o $(DEBUG_OUTPUT)
	./$(DEBUG_OUTPUT) DEBUG_ARG
	$(PROFILER) $(DEBUG_OUTPUT) gmon.out > $(PROFILE_OUTPUT)

clean:
	del $(OUTPUT) gmon.out
