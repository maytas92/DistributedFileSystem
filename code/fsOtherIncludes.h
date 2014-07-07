/* 
 * Mahesh V. Tripunitara
 * University of Waterloo
 * You specify what goes in this file. I just have a "dummy"
 * specification of the FSDIR type.
 */

#ifndef _ECE_FS_OTHER_INCLUDES_
#define _ECE_FS_OTHER_INCLUDES_
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>

/*
* Linked list of clients that have mounted the server
*/
struct client {
	uint32_t clientIP;
	char *localFolderName;
	struct client * next;
};

typedef struct FSDIR {
	DIR *dir;
	// assume that the length of the folder name does not exceed 256 bytes
	char name[256]; 
	//struct dirent entry;
	//struct client who;
}FSDIR;



/* 
* Linked list of servers that have been mounted on a client
*/
struct mounted_servers {
	char * srvIpOrDomName;
	int srvPort;
	char * localFolderName;
	struct mounted_servers *next;
};

#endif
