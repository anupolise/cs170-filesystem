#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"

#define FILE_COUNT	64
#define BLOCK_SIZE	4096
#define BLOCK_COUNT	4096
#define DESC_COUNT	32
#define DISK_TITLE	"disk.txt"

struct File {
	char* filename;
	int startblock;
	int permission;
};

struct FD {
	int status;
	int startblock;
	int offset;
	char* filename;
};


// Global variables
struct FD FDS[DESC_COUNT];
struct File DIR[FILE_COUNT];
int FAT[BLOCK_COUNT];

// // Global counters
// int fat_block_counter = 0;
// int directory_counter = 0;
// int file_desc_counter = 0;


int make_fs(char* disk_name);
int mount_fs(char* disk_name);
int umount_fs(char* disk_name);
int fs_open(char* filename);
int fs_close(int fd);
int fs_create(char* filename);
int fs_delete(char* filename);
int fs_read(int fildes, void *buf, size_t nbyte);

int get_start_block(char* filename);
int get_available_block();
int len(char* string);


int make_fs(char *disk_name) {
	int success = make_disk(disk_name);

	if (success >= 0) {
		mount_fs(disk_name);

		// Metadata: set Directory values to default
		for (int i = 0; i < FILE_COUNT; i++) {
			DIR[i].filename = NULL;
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

	// Metadata: set Directory values to default
	for (int i = 0; i < DESC_COUNT; i++) {
		FDS[i].startblock = -1;
		FDS[i].status = -1;
		FDS[i].offset = -1;
		FDS[i].filename = NULL;
	}

	free(buffer);
	return 0;
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
	return 0;
}

int fs_open(char* filename) {
	int startblock = get_start_block(filename);

	// Error if file doesn't exists
	if (startblock == -1) return -1;

	for (int i = 0; i < DESC_COUNT; i++) {
		if (FDS[i].startblock == -1) {
			FDS[i].startblock = startblock;
			FDS[i].offset = 0;
			FDS[i].status = 0;
			FDS[i].filename = filename;
			return i;
		}
	}

	printf("Too many files open.\n");
	return -1;
}

int fs_close(int fd) {
	if (FDS[fd].status == -1)
		return -1;

	FDS[fd].startblock = -1;
	FDS[fd].offset = -1;
	FDS[fd].status = -1;
	FDS[fd].filename = NULL;
	return 0;
}

int fs_create(char* filename) {
	// Error if filename is too long or file already exists in DIR
	if (len(filename) > 15 || get_start_block(filename) != -1)
		return -1;

	for (int i = 0; i < FILE_COUNT; i++) {
		if (DIR[i].filename == NULL) {
			int available_block = get_available_block();
			printf("availdable block: %d\n", available_block);
			if (available_block == -1) return -1;

			DIR[i].filename = filename;
			DIR[i].startblock = available_block;
			DIR[i].permission = 0;
			FAT[available_block] = -2;

			//----------testing----------
			char* buffer = malloc(BLOCK_SIZE);
			strncpy(buffer, "hello123", 8);
			block_write(available_block, buffer);
			free(buffer);
			//----------testing----------

			return 0;
		}
	}
	return -1;
}

int fs_delete(char* filename) {
	for (int i = 0; i < DESC_COUNT; i++)
		if (FDS[i].filename != NULL && strcmp(FDS[i].filename, filename) == 0)
			return -1;

	int nextblock = get_start_block(filename); 
	if (nextblock == -1)
		return -1;

	char* buffer = malloc(BLOCK_SIZE);
	memset(buffer, 0, sizeof(buffer));

	while (nextblock != -2 && nextblock != -1) {
		block_write(nextblock, buffer);
		int temp = nextblock;
		nextblock = FAT[nextblock];
		FAT[temp] = -1;
	}

	return 0;
}

int fs_read(int filedes, void *buf, size_t nbyte){
	printf("1\n");
	if(FDS[filedes].status == -1)
		return -1;

	printf("1\n");
	void *bufptr = buf;
	if(FDS[filedes].offset!=0){
		char* temp_buf = malloc(BLOCK_SIZE);
		block_read(FDS[filedes].startblock, temp_buf);
		memcpy(bufptr, temp_buf+FDS[filedes].offset, BLOCK_SIZE-FDS[filedes].offset);
		nbyte-= BLOCK_SIZE-FDS[filedes].offset;
		bufptr+=BLOCK_SIZE-FDS[filedes].offset;
	}
	printf("2\n");

	while(nbyte>BLOCK_SIZE && FDS[filedes].startblock!=-1 && FDS[filedes].startblock!=-2){
		block_read(FDS[filedes].startblock, bufptr);
		nbyte-=BLOCK_SIZE;
		FDS[filedes].startblock = FAT[FDS[filedes].startblock];
		bufptr+=BLOCK_SIZE;
	}
	printf("3\n");

	if(FDS[filedes].startblock >= 0){
		block_read(FDS[filedes].startblock, bufptr);
		FDS[filedes].offset = nbyte;
	}
	
	
}

// get string length
int len(char* string) {
	// int count = 0;
	// while (string[count++] != '\0');
	// return count - 1;
	return 0;
}


// get start block of file
int get_start_block(char* filename) {
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename != NULL && strcmp(DIR[i].filename, filename) == 0)
			return DIR[i].startblock;

	printf("File not exist.\n");
	return -1;
}

// get next available block in FAT
int get_available_block() {
	for (int i = 0 ; i < BLOCK_COUNT; i++) {
		if (FAT[i] == -1) {
			return i;
		}
	}

	printf("No available block.\n");
	return -1;
}


int main() {
	make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);


	//-----------------------------------Changes--------------------------------
	
	fs_create("hello.txt");
	fs_create("bye.txt");
	fs_create("checkstringlargerthan15");

	int fd = fs_open("hello.txt");
	printf("helo\n");
	char* buffer = malloc(BLOCK_SIZE);
	fs_read(fd, buffer, 3);
	printf("first string: %s\n", buffer);
	fs_read(fd, buffer, 3);
	printf("seocnd string: %s\n", buffer);
	


	// fs_delete("hello.txt");

	// block_read(0, buffer);

	//-----------------------------------Changes--------------------------------


	// umount_fs(DISK_TITLE);
	// mount_fs(DISK_TITLE);


	//-----------------------------------Testing--------------------------------
	printf("FAT 0: %d\n", FAT[0]);
	printf("FAT 1: %d\n", FAT[1]);
	printf("FAT 2: %d\n", FAT[2]);
	printf("FAT 3: %d\n", FAT[3]);
	// printf("DIR 0: %s, %d, %d\n", DIR[0].filename, DIR[0].startblock, DIR[0].permission);
	// printf("DIR 1: %s, %d, %d\n", DIR[1].filename, DIR[1].startblock, DIR[1].permission);
	// printf("DIR 2: %s, %d, %d\n", DIR[2].filename, DIR[2].startblock, DIR[2].permission);
	// printf("DIR 3: %s, %d, %d\n", DIR[3].filename, DIR[3].startblock, DIR[3].permission);
	// printf("FD 0: %d\n", FDS[0].startblock);
	// printf("FD 1: %d\n", FDS[1].startblock);
	// printf("FD 2: %d\n", FDS[2].startblock);

	//-----------------------------------Testing--------------------------------

	return 0;
}
