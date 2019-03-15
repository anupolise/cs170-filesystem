#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <setjmp.h> 	/* for performing non-local gotos with setjmp/longjmp */
#include <sys/time.h> 	/* for setitimer */
#include <signal.h> 	/* for sigaction */
#include <unistd.h> 	/* for pause */
#include "disk.h"

#define MAX_FILE_COUNT 	64
#define BLOCK_SIZE 		4096
#define ENTRIES 		4096
#define DISK_TITLE		"disk.txt"

struct DirectoryEntry {
	char* fileName;
	int startBlock;
	int permission;
};

// Global variables
struct DirectoryEntry directory[MAX_FILE_COUNT];
int FAT[ENTRIES];


// Function definitions
int make_fs(char *disk_name) {
	return make_disk(disk_name);
}

int mount_fs(char *disk_name) {
	open_disk(disk_name);
	char* metadata = malloc(BLOCK_SIZE);
	block_read(0,metadata);

	for(int i = 0; i<MAX_FILE_COUNT; i++){
		directory[i].fileName = (char*)metadata+i*12;
		directory[i].startBlock = *(int*)metadata+i*12+4;
		directory[i].permission = *(int*)metadata+i*12+8;
	}

	for(int i=1; i<5; i++){
		block_read(i,metadata);
		for(int j=0; j<1024; j++){
			FAT[(i-1) * 1024 + j] = (int)metadata[j*4];
		}
	}
}

int umount_fs(char *disk_name) {
		char* bufBytes = malloc(BLOCK_SIZE);

		for(int i=0; i<MAX_FILE_COUNT; i++){
			if (directory[i].fileName != NULL)
				memcpy(bufBytes+i*12, directory[i].fileName,4);

			memcpy(bufBytes+i*12+4, &directory[i].startBlock, 4);
			memcpy(bufBytes+i*12+8, &directory[i].permission, 4);
		}

		int counterBlock = 0;
		for(int i=0; i<ENTRIES; i++){
			if((i*4)%4096 == 0) {
				block_write(counterBlock, bufBytes);
				counterBlock++;
			}
			memcpy(bufBytes+((i*4)%4096), &FAT[i], 4);
		}
		close_disk();
}

int main () {
	make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);

	directory[0].fileName = "123";
	directory[0].startBlock = 10;
	directory[0].permission = 9;

	directory[1].fileName = "456";
	directory[1].startBlock = 756;
	directory[1].permission = 12324;

	umount_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);

	while(1);
	return 0;
}
