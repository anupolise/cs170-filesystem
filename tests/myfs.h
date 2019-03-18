#ifndef _FS_H_
#define _FS_H_
#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"

#define FILE_COUNT	64
#define BLOCK_SIZE	4096
#define BLOCK_COUNT	4096
#define DESC_COUNT	32

struct File {
	char* filename;
	int startblock;
	int permission;
	int finaloffset;
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


int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);

int fs_open(char *name);
int fs_close(int fildes);
int fs_create(char *name);
int fs_delete(char *name);
int fs_read(int fildes, void *buf, size_t nbyte);
int fs_write(int fildes, void *buf, size_t nbyte);
int fs_get_filesize(int fildes);
int fs_lseek(int fildes, off_t offset);
int fs_truncate(int fildes, off_t length);

//helper functions
int get_start_block(char* filename);
int get_available_block();
int len(char* string);

#endif