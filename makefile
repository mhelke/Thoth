all: 
	gcc -oFast *.c -o thoth.exe
debug:
	gcc *.c -o thoth.exe