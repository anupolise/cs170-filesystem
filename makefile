all: compile test

compile:
	gcc -c -o disk.o disk.c -m32

test:
	gcc -o file_system main.c disk.o -m32

clean:
	rm *.o
	rm file_system

