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
	for (int i = 0; i < MAX_FILE_COUNT; i++) {
		directory[i].fileName = NULL;
		directory[i].startBlock = -1;
		directory[i].permission = 0;
	}

	for (int i = 0; i < ENTRIES; i++) {
		FAT[i].block = NULL;
		FAT[i].next = -1;
	}

	// read metadata from disk
}

int umount_fs(char *disk_name) {
	for (int i = 0; i < MAX_FILE_COUNT; i++) {
		directory[i].fileName = NULL;
		directory[i].startBlock = -1;
		directory[i].permission = 0;
	}

	for (int i = 0; i < ENTRIES; i++) {
		free (FAT[i].block);
		FAT[i].next = -1;
	}
}

int main () {
	make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);
	return 0;
}
