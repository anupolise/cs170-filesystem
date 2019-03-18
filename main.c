#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"       /* for disk */
#include "fs.h"         /* file system */

#define DISK_TITLE "disk.txt"
#define TEST_FILE1 "hello1.txt"

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


// Test: write < 1 block + no offset
void test_write1() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(4096);

	for (int i = 0; i < 4096; i++) {
        if (i < 10)
            strncpy(buffer + i, "a", 1);
        else
            strncpy(buffer + i, "b", 1);
    }

    fs_write(fd, buffer, 10);
    block_read(0, buffer);
    printf("Test Write 1: %s", buffer);

	fs_write(fd, buffer + 10, 4086);
    block_read(0, buffer);
    printf("Test Write 1: %s", buffer);

    free(buffer);
}

void test_write2() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(6144);
	for (int i = 0; i < 6144; i++) {
		if (i < 4096)
			strncpy(buffer + i, "a", 1);
		else
			strncpy(buffer + i, "b", 1);
	}
}

void test_write3() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(6144);
}

void test_write4() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(6144);
}

int main(){
    make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);

	fs_create(TEST_FILE1);
	fs_create("bye.txt");
	fs_create("checkstringlargerthan15");

	// int fd1 = fs_open("hello.txt");
	// int fd2 = fs_open("hello.txt");


	// fs_write(fd1, buffer, 6144);

	// memset(buffer, 0, 6144);
	// block_read(0, buffer);
	// printf("block string: %s\n", buffer);
	// printf("FAT: %d\n", FAT[0]);

	// memset(buffer, 0, 6144);
	// block_read(FAT[0], buffer);
	// printf("block string: %s\n", buffer);

	// //add cs
	// for(int i = 0; i<15; i++){
	// 	strncpy(buffer+i, "c", 1);
	// }

	// printf("FD 0: %d, %d\n", FDS[0].startblock, FDS[0].offset);

	// fs_write(fd1, buffer, 15);
	// printf("FD 0: %d, %d\n", FDS[0].startblock, FDS[0].offset);

	// memset(buffer, 0, 6144);
	// block_read(FAT[0], buffer);
	// printf("block string: %s\n", buffer);

	// memset(buffer, 0, 6144);
	// fs_read(fd2, buffer, 4096);
	// printf("first string: %s\n", buffer);

	// memset(buffer, 0, 6144);
	// fs_read(fd2, buffer, 6144 - 4096);
	// printf("seocnd string: %s\n", buffer);
	


	// fs_delete("hello.txt");

	// block_read(0, buffer);

	//-----------------------------------Changes--------------------------------


	// umount_fs(DISK_TITLE);
	// mount_fs(DISK_TITLE);


}
