#include "fs.h"

int make_fs(char *disk_name) {
	int success = make_disk(disk_name);

	if (success >= 0) {
		mount_fs(disk_name);

		// Metadata: set Directory values to default
		for (int i = 0; i < FILE_COUNT; i++) {
			DIR[i].filename[0] = '\0';
			DIR[i].startblock = -1;
			DIR[i].permission = 0;
			DIR[i].finaloffset = 0;
		}

		// Metadata: set FAT values to default
		for (int i = 0; i < BLOCK_COUNT; i++)
			FAT[i] = -1;

		umount_fs(disk_name);
	}

	return success;
}

int mount_fs(char *disk_name) {
	int res = open_disk(disk_name);
	if (res < 0) return res;

	char* buffer = malloc(BLOCK_SIZE);

	// load DIR metadata
	char* buffer1 = buffer;
	memset(buffer, 0, BLOCK_SIZE);
	block_read(4096, buffer);
	for (int i = 0; i < FILE_COUNT; i++) {
		memcpy(DIR[i].filename, buffer + i * 27, 15);
		DIR[i].startblock = *(int*)(buffer + i * 27 + 15);
		DIR[i].permission = *(int*)(buffer + i * 27 + 19);
		DIR[i].finaloffset = *(int*)(buffer + i * 27 + 23);
	}

	char* testbuf = malloc(BLOCK_SIZE);
	block_read(0, testbuf);


	// load FAT metadata
	for(int i = 0; i < 4; i++){
		memset(buffer, 0, BLOCK_SIZE);
		block_read(i + 4097, buffer);
		memcpy(FAT + i * BLOCK_SIZE / 4, buffer, BLOCK_SIZE);
	}

	// Metadata: set Directory values to default
	for (int i = 0; i < DESC_COUNT; i++) {
		FDS[i].startblock = -1;
		FDS[i].status = -1;
		FDS[i].offset = -1;
		FDS[i].filename[0] = '\0';
	}

	free(buffer);
	return res;
}

int umount_fs(char *disk_name) {
	char* buffer = malloc(BLOCK_SIZE);

	// store DIR metadata
	memset(buffer, 0, BLOCK_SIZE);
	for (int i = 0; i < FILE_COUNT; i++) {
		for (int k = 0; k < 15; k++)
			strncpy(buffer + i * 27 + k, DIR[i].filename + k, 1);
		memcpy(buffer + i * 27 + 15, (char*) &DIR[i].startblock, 4);
		memcpy(buffer + i * 27 + 19, (char*) &DIR[i].permission, 4);
		memcpy(buffer + i * 27 + 23, (char*) &DIR[i].finaloffset, 4);
	}
	block_write(4096, buffer);

	// store FAT metadata
	for(int i = 0; i < 4; i++) {
		memset(buffer, 0, BLOCK_SIZE);
		memcpy(buffer, (char*)FAT + i * BLOCK_SIZE, BLOCK_SIZE);
		block_write(i + 4097, buffer);
	}

	free(buffer);
	return close_disk();
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
			strcpy(FDS[i].filename, filename);
			return i;
		}
	}

	// printf("Too many files open.\n");
	return -1;
}

int fs_close(int fd) {
	if (FDS[fd].status == -1)
		return -1;

	FDS[fd].startblock = -1;
	FDS[fd].offset = -1;
	FDS[fd].status = -1;
	FDS[fd].filename[0] = '\0';
	return 0;
}

int fs_create(char* filename) {
	// Error if filename is too long or file already exists in DIR
	if (len(filename) > 15 || get_start_block(filename) != -1)
		return -1;


	for (int i = 0; i < FILE_COUNT; i++) {
		if (DIR[i].startblock == -1) {
			int available_block = get_available_block();
			if (available_block == -1) return -1;

			strcpy(DIR[i].filename, filename);
			DIR[i].startblock = available_block;
			DIR[i].permission = 0;
			DIR[i].finaloffset = 0;
			FAT[available_block] = -2;
			return 0;
		}
	}
	return -1;
}

int fs_delete(char* filename) {
	char strbuf[15];
	strcpy(strbuf, filename);

	//check for any open file desc
	for (int i = 0; i < DESC_COUNT; i++){
		if (strcmp(FDS[i].filename, strbuf) == 0)
			return -1;
	}
		
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

	//removes file entry from DIR 
	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, filename) == 0)
			fileindex = i;
	if (fileindex < 0) return -1;
	DIR[fileindex].filename[0] = '\0';
	DIR[fileindex].startblock = -1;
	DIR[fileindex].permission = 0;
	DIR[fileindex].finaloffset = 0;

	return 0;
}


int fs_read(int filedes, void *buf, size_t nbyte) {
	//if fd is not open, negative or more than 32 fds open --> return
	if (filedes < 0 || filedes >= 32 || FDS[filedes].status == -1) return -1;

	//find corresponding DIR entry
	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, FDS[filedes].filename) == 0)
			fileindex = i;
	if (fileindex < 0) return -1;

	void* buf_block = buf;
	void* temp_buf = malloc(BLOCK_SIZE);

	int totalread = 0;
	int byteread = BLOCK_SIZE - FDS[filedes].offset; //how many bytes to read on currblock
	if (nbyte < byteread) byteread = nbyte;

	int currblock = FDS[filedes].startblock;
	int currOffset = FDS[filedes].offset;
	int done = 0;

	while (!done) {
		memset(temp_buf, 0, BLOCK_SIZE);

		// handle last block
		if (FAT[currblock] < 0) {
			if (nbyte > DIR[fileindex].finaloffset - currOffset) {
				nbyte = DIR[fileindex].finaloffset - currOffset;
				byteread = nbyte;
			}
		}

		// read current block and combine new buffer data
		block_read(currblock, temp_buf);
		memcpy(buf_block, temp_buf + currOffset, byteread);

		// set counters and offsets for next iteration
		totalread += byteread;
		nbyte -= byteread;
		buf_block = buf_block + byteread;

		if (nbyte >= BLOCK_SIZE) {
			byteread = BLOCK_SIZE;
		}
		else {
			FDS[filedes].offset = currOffset + byteread;
			byteread = nbyte;
		}

		currOffset = 0;
		if (byteread == 0) {
			done = 1;
			continue;
		}

		currblock = FAT[currblock];
		FDS[filedes].startblock = currblock;
	}

	return totalread;
}

int fs_write(int filedes, void *buf, size_t nbyte) {
	if (filedes < 0 || filedes >= 32 || FDS[filedes].status == -1) return -1;

	//find corresponding DIR entry
	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, FDS[filedes].filename) == 0)
			fileindex = i;
	if (fileindex < 0) return -1;

	void* buf_block = buf;
	void* temp_buf = malloc(BLOCK_SIZE);
	int bytewrite = BLOCK_SIZE - FDS[filedes].offset;
	if (nbyte < bytewrite) bytewrite = nbyte;

	int currblock = FDS[filedes].startblock;
	int currOffset = FDS[filedes].offset;
	int done = 0;
	int totalwrite = 0;

	while (!done) {
		memset(temp_buf, 0, BLOCK_SIZE);

		// read current block and combine new buffer data
		block_read(currblock, temp_buf);
		memcpy(temp_buf + currOffset, buf_block, bytewrite);

		// write data to block
		block_write(currblock, temp_buf);

		// set counters and offsets for next iteration
		totalwrite += bytewrite;
		nbyte -= bytewrite;
		buf_block = buf_block + bytewrite;

		if (nbyte >= BLOCK_SIZE) {
			bytewrite = BLOCK_SIZE;
		}
		else {
			FDS[filedes].offset = currOffset + bytewrite;
			bytewrite = nbyte;
		}

		currOffset = 0;
		if (bytewrite == 0) {
			done = 1;
			continue;
		}

		// expand file blocks if necessary
		if (FAT[currblock] < 0) {
			int temp  = currblock;
			currblock = get_available_block();
			if (currblock == -1) return totalwrite;
			FAT[temp] = currblock;
			FAT[currblock] = -2;
			DIR[fileindex].finaloffset = 0;
		}
		else {
			currblock = FAT[currblock];
		}
	}

	FDS[filedes].startblock = currblock;
	if (FAT[currblock] < 0)
		DIR[fileindex].finaloffset = (DIR[fileindex].finaloffset > FDS[filedes].offset) ? DIR[fileindex].finaloffset : FDS[filedes].offset;

	return totalwrite;
}


int fs_get_filesize(int filedes) {
	if (filedes < 0 || filedes >= 32 || FDS[filedes].status == -1) return -1;
	
	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, FDS[filedes].filename) == 0)
			fileindex = i;
	
	int currblock = DIR[fileindex].startblock;
	int totalbytes = 0;
	while(FAT[currblock] > 0){
		totalbytes+=BLOCK_SIZE;
		currblock = FAT[currblock];
	}

	totalbytes += DIR[fileindex].finaloffset;
	return totalbytes;
}

int fs_lseek(int filedes, off_t offset) {
	if (filedes < 0 || filedes >= 32 || FDS[filedes].status == -1) return -1;
	if(offset<0) return -1;

	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, FDS[filedes].filename) == 0)
			fileindex = i;
	
	int currblock = DIR[fileindex].startblock;

	while(FAT[currblock] > 0){
		if(offset < BLOCK_SIZE)
			break;
		offset -= BLOCK_SIZE;
		currblock = FAT[currblock];
	}

	if(FAT[currblock] < 0 && offset > DIR[fileindex].finaloffset)
		return -1;

	FDS[filedes].startblock = currblock;
	FDS[filedes].offset = offset;
	return 0;
}

int fs_truncate(int filedes, off_t length) {
	if (filedes < 0 || filedes >= 32 || FDS[filedes].status == -1) return -1;
    
    //if length greater than file size, jump to end 
    if(length > fs_get_filesize(filedes)){
        fs_lseek(filedes, fs_get_filesize(filedes));
        return -1;
    }

   	int res = fs_lseek(filedes, length);
	if(res == -1) return -1; //length truncate is larger than filesize

	//truncates the new last block
    char* buffer = malloc(BLOCK_SIZE);
    char* buf = malloc(BLOCK_SIZE);
    memset(buffer, 0, BLOCK_SIZE);
    memset(buf, 0, BLOCK_SIZE);
    block_read(FDS[filedes].startblock, buffer);
    strncpy(buf, buffer, FDS[filedes].offset);
    block_write(FDS[filedes].startblock, buf);

	//if currblock(new last block) is EOF, return
    int currblock =  FDS[filedes].startblock;
    if(currblock == -2){
        return 0;
    }

	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (strcmp(DIR[i].filename, FDS[filedes].filename) == 0)
			fileindex = i;
	if (fileindex == -1) return -1;

	DIR[fileindex].finaloffset = FDS[filedes].offset;

	//loop - free all blocks after new last block
    int temp = FAT[currblock];
    FAT[currblock] = -2;
    currblock = temp;
    while(currblock >= 0){
        memset(buffer, 0, BLOCK_SIZE);
        block_write(currblock, buffer);
        temp = FAT[currblock];
        FAT[currblock] = -1;
        currblock = temp;
    } 
    if(currblock!=-2){
        memset(buffer, 0, BLOCK_SIZE);
        block_write(currblock, buffer);
        temp = FAT[currblock];
        FAT[currblock] = -1;
        currblock = temp;

    }

    return 0;
}


// get string length
int len(char* string) {
	int count = 0;
	while (string[count++] != '\0');
	return count - 1;
	return 0;
}


// get start block of file
int get_start_block(char* filename) {
	char strbuf[15];
	strcpy(strbuf, filename);

	for (int i = 0; i < FILE_COUNT; i++) {
		if (strcmp(DIR[i].filename, strbuf) == 0)
			return DIR[i].startblock;
	}

	// printf("File not exist.\n");
	return -1;
}

// get next available block in FAT
int get_available_block() {
	for (int i = 0 ; i < BLOCK_COUNT; i++) {
		if (FAT[i] == -1) {
			return i;
		}
	}

	// printf("No available block.\n");
	return -1;
}
