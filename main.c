#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"

#define FILE_COUNT	64
#define BLOCK_SIZE	4096
#define BLOCK_COUNT	4096
#define DISK_TITLE	"disk.txt"

struct File {
	char* filename;
	int startblock;
	int permission;
};

// Global variables
struct File DIR[FILE_COUNT];
int FAT[BLOCK_COUNT];


int make_fs(char* disk_name);
int mount_fs(char* disk_name);
int umount_fs(char* disk_name);


int make_fs(char *disk_name) {
	int success = make_disk(disk_name);

	if (success >= 0) {
		mount_fs(disk_name);

		// Metadata: set Directory values to default
		for (int i = 0; i < FILE_COUNT; i++) {
			DIR[i].filename = "";
			DIR[i].startblock = -1;
			DIR[i].permission = 0;
		}

		// Metadata: set FAT values to default
		for (int i = 0; i < BLOCK_COUNT; i++)
			FAT[i] = -1;

		umount_fs(disk_name);
	}

	return success;
}

int mount_fs(char *disk_name) {
	open_disk(disk_name);
	char* buffer = malloc(BLOCK_SIZE);

	// load DIR metadata
	memset(buffer, 0, BLOCK_SIZE);
	block_read(0, buffer);
	memcpy(DIR, buffer, sizeof(DIR));

	// load FAT metadata
	for(int i = 0; i < 4; i++){
		memset(buffer, 0, BLOCK_SIZE);
		block_read(i + 1, buffer);
		memcpy(FAT + i * BLOCK_SIZE / 4, buffer, BLOCK_SIZE);
	}

	free(buffer);
}

int umount_fs(char *disk_name) {
	char* buffer = malloc(BLOCK_SIZE);

	// store DIR metadata
	memset(buffer, 0, BLOCK_SIZE);
	memcpy(buffer, DIR, sizeof(DIR));
	block_write(0, buffer);

	// store FAT metadata
	for(int i = 0; i < 4; i++) {
		memset(buffer, 0, BLOCK_SIZE);
		memcpy(buffer, (char*)FAT + i * BLOCK_SIZE, BLOCK_SIZE);
		block_write(i + 1, buffer);
	}

	free(buffer);
	close_disk();
}



int main () {
	make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);


	//-----------------------------------Changes--------------------------------
	for (int i = 0; i < 3; i++)
		FAT[i] = 5;

	DIR[0].filename = "dog.txt";
	DIR[0].startblock = 0;
	DIR[0].permission = 1;

	DIR[1].filename = "cat.py";
	DIR[1].startblock = 7;
	DIR[1].permission = 2;
	//-----------------------------------Changes--------------------------------


	umount_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);


	//-----------------------------------Testing--------------------------------
	printf("FAT 0: %d\n", FAT[0]);
	printf("FAT 1: %d\n", FAT[1]);
	printf("FAT 2: %d\n", FAT[2]);
	printf("FAT 3: %d\n", FAT[3]);
	printf("DIR 0: %s, %d, %d\n", DIR[0].filename, DIR[0].startblock, DIR[0].permission);
	printf("DIR 1: %s, %d, %d\n", DIR[1].filename, DIR[1].startblock, DIR[1].permission);
	printf("DIR 2: %s, %d, %d\n", DIR[2].filename, DIR[2].startblock, DIR[2].permission);
	printf("DIR 3: %s, %d, %d\n", DIR[3].filename, DIR[3].startblock, DIR[3].permission);
	//-----------------------------------Testing--------------------------------

	return 0;
}
