all: compile test

compile:
	gcc -c -o disk.o disk.c -m32 -g
	gcc -c -o fs.o fs.c -m32 -g

test:
	gcc -o file_system main.c disk.o fs.o -m32 -g

clean:
	rm *.o
	rm file_system

