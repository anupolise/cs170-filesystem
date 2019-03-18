#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"       /* for disk */
#include "fs.h"         /* file system */

#define DISK_TITLE "disk.txt"
#define TEST_FILE1 "hello1.txt"
#define TEST_FILE2 "hello2.txt"

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
// Test: appending to a block (no overflow) + offset
void test_write1() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(4096);
    char* result_buf = malloc(4096);
	for (int i = 0; i < 4096; i++) {
        if (i < 10)
            strncpy(buffer + i, "a", 1);
        else
            strncpy(buffer + i, "b", 1);
    }

    fs_write(fd, buffer + 10, 4086);
	fs_write(fd, buffer, 10);
    block_read(0, result_buf);
    printf("Test Write 1: %s\n", result_buf);

    free(result_buf);   
    free(buffer);
}

//Test: writing more than one block + no offset
void test_write2() {
	int fd = fs_open(TEST_FILE2);
	char* buffer = malloc(6144);
	char* result_buf = malloc(6144);

	for (int i = 0; i < 6144; i++) {
		if (i < 4096)
			strncpy(buffer + i, "c", 1);
		else
			strncpy(buffer + i, "d", 1);
	}

    fs_write(fd, buffer, 6144);
    int curblock = get_start_block(TEST_FILE2);
    block_read(curblock, result_buf);
    printf("Test Write 2 (no offset): %s\n", result_buf);
    curblock = FAT[curblock];
    block_read(curblock, result_buf);
    printf("Test Write 2 (no offset) second block: %s\n", result_buf);
}

//Test: writing more than one block + offset 
void test_write3() {
	int fd = fs_open(TEST_FILE2);
	char* buffer = malloc(BLOCK_SIZE*3);
	char* result_buf = malloc(BLOCK_SIZE);

	for (int i = 0; i < 6144; i++) {
		if (i < 4096)
			strncpy(buffer + i, "c", 1);
		else
			strncpy(buffer + i, "d", 1);
	}

    fs_write(fd, buffer, 6144);

    for (int i = 0; i < BLOCK_SIZE*3; i++) {
		if (i < 4096)
			strncpy(buffer + i, "e", 1);
		else
			strncpy(buffer + i, "f", 1);
	}
    
    int curblock = FDS[fd].startblock;
    fs_write(fd, buffer, BLOCK_SIZE*3);
    block_read(curblock, result_buf);
    printf("Test Write 3 (offset e&f): %s\n", result_buf);
    while(curblock>=0){
        curblock = FAT[curblock];
        if (curblock < 0) continue;
        block_read(curblock, result_buf);
        printf("Test Write 3 (no offset) %d block: %s\n",curblock, result_buf);
    }

    free(buffer);
    free(result_buf);
}

// test extreme edge case
void test_write4() {
	int fd = fs_open(TEST_FILE1);
	char* buffer = malloc(BLOCK_SIZE);
	char* result_buf = malloc(BLOCK_SIZE);

    int status = fs_write(fd, buffer, 0);
    block_read(FDS[fd].startblock, result_buf);
    printf("Test Write 4: %s\n", result_buf);
    printf("Test Write 4 Success: %d\n", status);

    fs_close(fd);

    status = fs_write(fd, buffer, BLOCK_SIZE);
    printf("Test Write 4 Success: %d (Fd Closed)\n", status);

    free(buffer);
    free(result_buf);
}

// test read entire block
void test_read1() {
	int fd = fs_open(TEST_FILE1);
	char* result_buf = malloc(BLOCK_SIZE);

    fs_read(fd, result_buf, BLOCK_SIZE);
    printf("Test Read 1: %s\n", result_buf);

    memset(result_buf, 0, BLOCK_SIZE);
    printFDS(10);

    fs_read(fd, result_buf, BLOCK_SIZE);
    printf("Test Read 1: %s\n", result_buf);

    free(result_buf);
}

// test read one and half a block
void test_read2() {
	int fd = fs_open(TEST_FILE2);
	char* result_buf = malloc(BLOCK_SIZE);

    fs_read(fd, result_buf, 2048);
    printf("Test Read 2: %s\n", result_buf);

    // printFDS(10);

    memset(result_buf, 0, BLOCK_SIZE);

    fs_read(fd, result_buf, BLOCK_SIZE);
    printf("Test Read 2: %s\n", result_buf);

    free(result_buf);
}

int main(){
    make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);

	fs_create(TEST_FILE1);
    fs_create(TEST_FILE2);

    // test_write1();
    test_write2();
    // test_write3();
    // test_write4();
    // test_read1();
    test_read2();
    printFAT(10);
    printFDS(10);
    printDirectory(10);

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
