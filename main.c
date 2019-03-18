#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <string.h> 	/* for pause */
#include "disk.h"       /* for disk */
#include "fs.h"         /* file system */

#define DISK_TITLE "disk.txt"
#define TEST_FILE1 "hello1.txt"
#define TEST_FILE2 "hello2.txt"
#define TEST_FILE3 "hello3.txt"
#define TEST_FILE4 "hello4.txt"

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
void test_write1(int fd) {
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
void test_write2(int fd) {
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
void test_write3(int fd) {
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
void test_write4(int fd) {
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
void test_read1(int fd) {
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
void test_read2(int fd) {
	char* result_buf = malloc(BLOCK_SIZE);

    fs_read(fd, result_buf, 2048);
    printf("Test Read 2: %s\n", result_buf);

    // printFDS(10);

    memset(result_buf, 0, BLOCK_SIZE);

    fs_read(fd, result_buf, BLOCK_SIZE);
    printf("Test Read 2: %s\n", result_buf);

    free(result_buf);
}

void test_seek1(int fd){
	char* buffer = malloc(15);
	 
	strncpy(buffer, "gggggggggg", 10);
	fs_write(fd, buffer, 10);

	int fd2 = fs_open(TEST_FILE3);
	fs_lseek(fd2, fs_get_filesize(fd));
	strncpy(buffer, "h", 1);
	fs_write(fd2, buffer, 1);

	memset(buffer, 0, 15);
	fs_lseek(fd2, -11);
	fs_read(fd2, buffer, 11);
    printf("Test Seek 1: %s\n", buffer);
	
}

int main(){
    make_fs(DISK_TITLE);
	mount_fs(DISK_TITLE);

	fs_create(TEST_FILE1);
    fs_create(TEST_FILE2);
    fs_create(TEST_FILE3);
    int fd1 = fs_open(TEST_FILE1);
    int fd2 = fs_open(TEST_FILE2);
    int fd1_1 =  fs_open(TEST_FILE1);
    // int fd3 =  fs_open(TEST_FILE3);


    // test_write1(fd1);
    // test_write2(fd2);
    test_write3(fd2);
    // test_write4(fd2);
    // test_read1(fd1);
    // test_read2(fd2);
    // test_seek1(fd3);
    //fs_close(fd2);
    fs_close(fd1);

    printFAT(10);
    printFDS(10);
    printDirectory(10);
    
   // int res = fs_delete(TEST_FILE1);
    int res =  fs_truncate(fd2,4*4096+100);
    printf("\n\n\n restult: %d \n\n\n",res);
    printFAT(10);
    printFDS(10);
    printDirectory(10);
	// printf("seocnd string: %s\n", buffer);
	
	printf("file size hello1: %d\n", fs_get_filesize(fd1));
	// printf("file size hello2: %d\n", fs_get_filesize(fd2));



	// fs_delete("hello.txt");

	// block_read(0, buffer);

	//-----------------------------------Changes--------------------------------


	// umount_fs(DISK_TITLE);
	// mount_fs(DISK_TITLE);


}
