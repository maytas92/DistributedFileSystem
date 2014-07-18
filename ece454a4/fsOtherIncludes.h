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
* Used to keep track of open file FD's on the client side
*/
typedef struct clientFD {
	int fd;
	int ip; // client IP
	char localFolderName[256];
	struct clientFD *next;
}clientFD;

/*
* Used to keep track of open files on the server side
*/
typedef struct fileOpen {
	int mode;
	struct fileOpen *next;
	int fd;
	char name[256]; // client side name
}fileOpen;

/*
* Linked list of clients that have mounted the server
*/
struct client {
	uint32_t clientIP;
	char *localFolderName;
	struct client * next;
	fileOpen *fileOpenHead;
};

typedef struct FSDIR {
	DIR *dir;
	// assume that the length of the folder name does not exceed 256 bytes
	// TODO: Fix this
	char name[256]; 
	int errNo;
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

typedef struct fileOpenServerPath {
	// fd created when the server opened the file for 
	// THE ONE client
	int fd; 
	char name[256]; // server side name
	struct fileOpenServerPath *next;
}fileOpenServerPath;

#endif