## CS170 Project 4 - Filesystem
Anu Polisetty 8916967
Wei Tung Chen 8936502

### int make_fs(char *disk_name);
Initialize a new filesystem to a disk with disk_name. Initialize metadata for FAT and DIR tables.

### int mount_fs(char *disk_name);
Mount an existing filesystem. Load metadata and actual data from disk. Initialize file descriptor table.

### int umount_fs(char *disk_name);
Umount an existing filesystem. Store metadata and actual data from disk. 

### int fs_open(char *name);
Open a new file descriptor for an existing file in the filesystem.

### int fs_close(int fildes);
Close a new file descriptor for an existing file in the filesystem.

### int fs_create(char *name);
Create a new file in the filesystem and initialize DIR and FAT metadata.

### int fs_delete(char *name);
Delete a file in the filesystem and reset corresponding DIR and FAT metadata entry.

### int fs_read(int fildes, void *buf, size_t nbyte);
Read nbyte data bytes starting from the given file descriptor location to buffer.

### int fs_write(int fildes, void *buf, size_t nbyte);
Write nbyte data bytes starting from the given file descriptor location to buffer. Expand file if necessary.

### int fs_get_filesize(int fildes);
Get filesize in bytes for the file corresponding given file descriptor

### int fs_lseek(int fildes, off_t offset);
Set file descriptor to the give offset starting from the top of a file

### int fs_truncate(int fildes, off_t length);
Set file descriptor to the give offset starting from the top of a file and delete all following blocks and bytes after.

### int get_start_block(char* filename);
Get starting block of a file from DIR table. 

### int get_available_block();
Get teh next available block based on the FAT table entry being -1 (empty)

### int len(char* string);
Get string length