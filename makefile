all: compile test

compile:
	gcc -c -o disk.o disk.c -g
	gcc -c -o fs.o fs.c -g

test:
	gcc -o file_system main.c disk.o fs.o -g

clean:
	rm *.o
	rm file_system

