#include "fs.h"

int make_fs(char *disk_name) {
	int success = make_disk(disk_name);

	if (success >= 0) {
		mount_fs(disk_name);

		// Metadata: set Directory values to default
		for (int i = 0; i < FILE_COUNT; i++) {
			DIR[i].filename = NULL;
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
			if (available_block == -1) return -1;

			DIR[i].filename = filename;
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

// int fs_read(int filedes, void *buf, size_t nbyte){
// 	if(FDS[filedes].status == -1)
// 		return -1;

// 	void *bufptr = buf;
// 	char* temp_buf = malloc(BLOCK_SIZE);
// 	memset(temp_buf, 0, sizeof(temp_buf));


// 	// reading first block (starting offset)
// 	if(FDS[filedes].offset != 0) {
// 		if (FDS[filedes].offset == BLOCK_SIZE) {
// 			if (FAT[FDS[filedes].startblock] == -2) return -1;
// 			FDS[filedes].startblock = FAT[FDS[filedes].startblock];
// 			FDS[filedes].offset = 0;
// 		}

// 		block_read(FDS[filedes].startblock, temp_buf);

// 		if (nbyte <= BLOCK_SIZE - FDS[filedes].offset) {
// 			memcpy(bufptr, temp_buf + FDS[filedes].offset, nbyte);
// 			FDS[filedes].offset = FDS[filedes].offset + nbyte;
// 			return 0;
// 		}

// 		memcpy(bufptr, temp_buf + FDS[filedes].offset, BLOCK_SIZE - FDS[filedes].offset);
// 		nbyte = nbyte - (BLOCK_SIZE - FDS[filedes].offset);
// 		bufptr += BLOCK_SIZE - FDS[filedes].offset;

// 		if (FDS[filedes].offset == BLOCK_SIZE) {
// 			if (FAT[FDS[filedes].startblock] == -2) return -1;
// 			FDS[filedes].startblock = FAT[FDS[filedes].startblock];
// 			FDS[filedes].offset = 0;
// 		}
// 	}

// 	// reading entire blocks
// 	while(nbyte > BLOCK_SIZE && FDS[filedes].startblock != -1 && FDS[filedes].startblock != -2){
// 		block_read(FDS[filedes].startblock, bufptr);
// 		nbyte -= BLOCK_SIZE;
// 		FDS[filedes].startblock = FAT[FDS[filedes].startblock];
// 		bufptr += BLOCK_SIZE;
// 	}

// 	// reading last block (up to nbyte)
// 	memset(temp_buf, 0, sizeof(temp_buf));
// 	if(FDS[filedes].startblock >= 0){
// 		block_read(FDS[filedes].startblock, temp_buf);
// 		memcpy(bufptr, temp_buf, nbyte);
// 		FDS[filedes].offset = nbyte;
// 	}

// 	free(temp_buf);
// }

int fs_read(int filedes, void *buf, size_t nbyte) {
	if (FDS[filedes].status == -1) return -1;

	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename == FDS[filedes].filename)
			fileindex = i;
	if (fileindex < 0) return -1;

	void* buf_block = buf;
	void* temp_buf = malloc(BLOCK_SIZE);

	int totalread = 0;
	int byteread = BLOCK_SIZE - FDS[filedes].offset;
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
	if (FDS[filedes].status == -1) return -1;

	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename == FDS[filedes].filename)
			fileindex = i;
	if (fileindex < 0) return -1;

	void* buf_block = buf;
	void* temp_buf = malloc(BLOCK_SIZE);
	int bytewrite = BLOCK_SIZE - FDS[filedes].offset;
	if (nbyte < bytewrite) bytewrite = nbyte;

	int currblock = FDS[filedes].startblock;
	int currOffset = FDS[filedes].offset;
	int done = 0;
	int totalwrite = nbyte;

	while (!done) {
		memset(temp_buf, 0, BLOCK_SIZE);

		// read current block and combine new buffer data
		block_read(currblock, temp_buf);
		memcpy(temp_buf + currOffset, buf_block, bytewrite);

		// write data to block
		block_write(currblock, temp_buf);

		// set counters and offsets for next iteration
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
			if (currblock == -1) return -1;
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

	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename == FDS[filedes].filename)
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
	int fileindex = -1;
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename == FDS[filedes].filename)
			fileindex = i;
	
	int currblock = FDS[filedes].startblock;
	offset+= FDS[filedes].offset;

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

int fs_truncate(int fildes, off_t length) {
	return -1;
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
	for (int i = 0; i < FILE_COUNT; i++)
		if (DIR[i].filename != NULL && strcmp(DIR[i].filename, filename) == 0)
			return DIR[i].startblock;

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
