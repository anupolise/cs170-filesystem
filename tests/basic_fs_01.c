#include "myfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printFAT(int entries){
    for(int i = 0 ; i<entries; i++){
        printf("FAT %d: %d\n",i, FAT[i]);
    }
}

void printDirectory(int entries){
     for(int i = 0 ; i<entries; i++){
        printf("DIR %d: %s block:%d %d %d\n",i, DIR[i].filename, DIR[i].startblock, DIR[i].permission, DIR[i].finaloffset);
    }
}

void printFDS(int entries){
     for(int i = 0 ; i<entries; i++){
        printf("FDS %d: %s block:%d %d status:%d\n",i, FDS[i].filename, FDS[i].startblock, FDS[i].offset, FDS[i].status);
    }
}

int main() {
	int fildes = 0;
	int ret = 0;
	char disk_name[] = "fs1";
	char file_name[] = "file1";

	// Make filesystem.
	ret = make_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: make_fs failed\n");
	}

	// Mount filesystem.
	ret = mount_fs(disk_name);
	if(ret != 0) {
		printf("ERROR: mount_fs failed\n");
	}

	// create file
	ret = fs_create(file_name);
	if(ret != 0) {
		printf("ERROR: fs_create failed\n");
	}

	// open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// write to file
	char data[] = "This is my data";
	int len = strlen(data);
	ret = fs_write(fildes,data,len);
	if(ret != len) {
		printf("ERROR: fs_write failed to write correct number of bytes\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// re-open file
	fildes = fs_open(file_name);
	if(fildes < 0) {
		printf("ERROR: fs_open failed\n");
	}

	// read what we just wrote into buffer
	char buffer[4096];
	ret = fs_read(fildes,buffer,len);
	if(ret != len) {
		printf("ERROR: fs_read failed to read correct number of bytes\n");
	}

	// make sure what we read matches with what we wrote
	ret = strncmp(data,buffer,len);
	if(ret != 0) {
		printf("ERROR: data read does not match data written!\n");
	}

	// close file
	ret = fs_close(fildes);
	if(ret != 0) {
		printf("ERROR: fs_close failed\n");
	}

	// unmount file system
	ret = umount_fs(disk_name);
	if(ret < 0) {
		printf("ERROR: umount_fs failed\n");
	}

	// ret = mount_fs(disk_name);
	// printDirectory(10);
	// printFAT(10);
	// done!
	return 0;
}