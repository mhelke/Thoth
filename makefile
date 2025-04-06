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
	if exist $(OUTPUT) del $(OUTPUT)
	if exist $(DEBUG_OUTPUT) del $(DEBUG_OUTPUT)
	if exist gmon.out del gmon.out
	if exist $(PROFILE_OUTPUT) del $(PROFILE_OUTPUT) 