#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <setjmp.h> 	/* for performing non-local gotos with setjmp/longjmp */
#include <sys/time.h> 	/* for setitimer */
#include <signal.h> 	/* for sigaction */
#include <unistd.h> 	/* for pause */
 
#define MAX_FILE_COUNT 	64
#define BLOCK_SIZE 		4096

struct directoryEntry{
	char* fileName;
	int startBlock;
	int permission; 
};

struct FATEntry{
	char* block;
	int next;
};

struct directoryEntry directory[MAX_FILE_COUNT];
struct FATEntry FAT[4096];

int make_fs(char *disk_name);
int mount_fs(char *disk_name);
int umount_fs(char *disk_name);