#include <stdlib.h>		/* for exit() */
#include <stdio.h> 		/* standard buffered input/output */
#include <setjmp.h> 	/* for performing non-local gotos with setjmp/longjmp */
#include <sys/time.h> 	/* for setitimer */
#include <signal.h> 	/* for sigaction */
#include <unistd.h> 	/* for pause */

struct directoryEntry{
	char* fileName;
	int startBlock;
};