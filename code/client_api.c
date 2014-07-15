/*
 * ECE 454: Distributed Systems. 
 * University of Waterloo
 * Khushi Sohi
 * Satyam Gupta
 * CLIENT API for the Distributed File System
*/
#include "ece454_fs.h"
#include <string.h>
#include <sys/types.h>
#include "simplified_rpc/ece454rpc_types.h"
#include <stdint.h>

#if 1
#define _DEBUG_1_
#endif

#if 0
#define _DEBUG_2_
#endif

struct fsDirent dent;

// Keeps track of the current number of mounted remote servers
// Note even if the same server is mounted twice with a different
// 'myFolder' then this counter would be incremented
static int num_mounted_servers = 0;

struct mounted_servers * ms_head = NULL;

struct clientFD * clientFD_head = NULL;

return_type ans;

struct mounted_servers * getRemoteServer(const char * folderName) {
	// base check
	if(ms_head == NULL) {
		return NULL;
	}
	struct mounted_servers *tmp;
	for(tmp = ms_head; tmp != NULL; tmp = tmp->next) {
		if( !strcmp(tmp->localFolderName, folderName) ) {

			printf("Found remote server\n");
			return tmp;
		}
	}
	return NULL;
}

void freeMountedServer(struct mounted_servers *to_free) {
	free(to_free->srvIpOrDomName);
	free(to_free->localFolderName);
	free(to_free);
}

void addMountedServer(const char * srvIpOrDomName, int srvPort, const char * localFolderName) {
	struct mounted_servers *ms = (struct mounted_servers *)malloc(sizeof(struct mounted_servers));
	
	ms->next = ms_head;
	
	ms->srvIpOrDomName = calloc(strlen(srvIpOrDomName) + 1, sizeof(char));
	strcpy(ms->srvIpOrDomName, srvIpOrDomName);

#ifdef _DEBUG_2_
	printf("The mounted server name is %s\n", ms->srvIpOrDomName);
#endif
	ms->srvPort = srvPort;

#ifdef _DEBUG_2_
	printf("The mounted server port is %d\n", ms->srvPort);
#endif
	
	ms->localFolderName = calloc(strlen(localFolderName) + 1, sizeof(char));
	strcpy(ms->localFolderName, localFolderName);

#ifdef _DEBUG_2_
	printf("The mounted local folder name is %s\n", ms->localFolderName);
#endif
	ms_head = ms;
	
	num_mounted_servers++;
#ifdef _DEBUG_2_
	printf("The number of mounted servers now is %d\n", num_mounted_servers);
#endif
}

int removeMountedServer(const char *localFolderName) {

	// Basic sanity check
	if(num_mounted_servers == 0) {
#ifdef _DEBUG_1_
	// TODO: set correct errno
	printf("fsUnMount: ERROR - The number of servers that have been mounted is zero. \
		You must mount a server before unMounting it."); fflush(stdout);
#endif		
		return -1;
	}
	struct mounted_servers *tmp = ms_head;

	// Handles the case when the head of the linked list matches the 
	// local folder name. Note that if the size of the linked list is 
	// one, then we would like to return subsequently after this conditional.
	if(!strcmp(tmp->localFolderName, localFolderName)) {
#ifdef _DEBUG_2_
	printf("fsUnMount: Unmounting %s\n", localFolderName); fflush(stdout);
#endif		
		num_mounted_servers--;
		// update head
		ms_head = ms_head->next;
		// TODO: Into a function
		freeMountedServer(tmp);
		return 0;
	} 
	// Previously we checked for the head's folder name matching the input argument 
	// If this is not the case and the number of mounted servers is 1, then return
	// gracefully
	if(num_mounted_servers == 1) {
#ifdef _DEBUG_1_
	printf("fsUnMount: ERROR - Unmounting a server that has not been mounted.\n"); fflush(stdout);
#endif
		return -1;
	}

	// the size of the linked list is atleast 2 and the head did not match the local folder name
	// so we iterate through the linked list of mounted servers
	struct mounted_servers *prev = ms_head;
	for(tmp = ms_head->next; tmp != NULL; tmp = tmp->next, prev = tmp) {

		if( !strcmp(tmp->localFolderName, localFolderName) ) {
			num_mounted_servers--;
			prev->next = tmp->next;
			// TODO: Into a function
			freeMountedServer(tmp);
		
			return 0;
		}
	}
#ifdef _DEBUG_1_
	printf("fsUnMount: ERROR - Unmounting a server that has not been mounted.\n"); fflush(stdout);
#endif
	return -1;
}

/*
* Mounts the remote folder locally. 
* Returns 0 on success and -1 on failure. Errno is set appropriately.
* 
*/
int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	uint32_t clientIP = getPublicIPAddr();
#ifdef _DEBUG_1_
	printf("fsMount Client IP Address: %d\n", clientIP);
#endif

#ifdef _DEBUG_2_
	printf("fsMount Client: %s\n", localFolderName); fflush(stdout);
#endif

	// Begin Client side storage of mounted servers
	addMountedServer(srvIpOrDomName, srvPort, localFolderName);
 	// End of client side storage

 	ans = make_remote_call(srvIpOrDomName, srvPort, "fsMount_remote", 2, 
 			strlen(localFolderName) + 1, localFolderName,
 			sizeof(clientIP), (void *)&clientIP );

#ifdef _DEBUG_2_
 	printf("fsMount Client: Return val: ", ans.return_val);
#endif 	
 	return *(int *)ans.return_val;
}

/*
* Unmounts a remote file system. 
* Returns 0 on success, -1 on failure. Errno is set appropriately.
*/
int fsUnMount(const char *localFolderName) {
	// remove the mounted server corresponding to localfoldername from the 'mounted servers'
	struct mounted_servers* rem_server = getRemoteServer(localFolderName);
	uint32_t clientIP = getPublicIPAddr();

	int s = 0;
	if(rem_server == NULL) {
#ifdef _DEBUG_1_
	printf("FS Unmount: Could not fetch remote server belonging to folder name\n"); fflush(stdout);
#endif
		return -1;		
	}

	ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsUnMount_remote", 2, 
			strlen(rem_server->localFolderName) + 1, rem_server->localFolderName,
			sizeof(clientIP), (void *)&clientIP);

	int servUnMount = *(int *)ans.return_val;
	
	// if success
	if(servUnMount == 0) {
		return removeMountedServer(localFolderName);
	}

	// error unmounting on server side
#ifdef _DEBUG_1_
	printf("FS Unmount: Error unmounting on server side\n");
#endif
	return -1;
}

FSDIR* fsOpenDir(const char *folderName) {
	uint32_t clientIP = getPublicIPAddr();
	char * tmpFolderName = malloc(strlen(folderName) + 1);
	strcpy(tmpFolderName, folderName);
	char *localFolderName = strtok(tmpFolderName, "/");
	if(localFolderName == NULL) {
		strcpy(localFolderName, folderName);
	}

#ifdef _DEBUG_1_
	printf("FS Open Dir:\n"); fflush(stdout);
	printf("FS Open Dir %s:\n", localFolderName); fflush(stdout);
#endif

	struct mounted_servers* rem_server = getRemoteServer(localFolderName);
	if( rem_server == NULL) {
#ifdef _DEBUG_1_
	printf("FS Open Dir: Could not fetch remote server belonging to folder name\n"); fflush(stdout);
#endif
		return NULL;
	}
	ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsOpenDir_remote", 2, 
 			strlen(folderName) + 1, folderName,
 			sizeof(clientIP), (void *)&clientIP);
    FSDIR *ans_ret = (FSDIR *)ans.return_val;
	printf("The folder name opened on the client side%s\n", ans_ret->name);
    return (FSDIR *)ans.return_val;
}

int fsCloseDir(FSDIR *folder) {

#ifdef _DEBUG_1_
	printf("FS Close Dir:\n"); fflush(stdout);
#endif
	// http://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html
	char *folderName = folder->name;
	char * tmpFolderName = malloc(strlen(folderName) + 1);
	strcpy(tmpFolderName, folderName);
	char *localFolderName = strtok(tmpFolderName, "/");
	if(localFolderName == NULL) {
		strcpy(localFolderName, folderName);
	}

#ifdef _DEBUG_1_
	printf("FS Close Dir %s:\n", folderName); fflush(stdout);
#endif
	
	struct mounted_servers *rem_server = getRemoteServer(localFolderName);
	if( rem_server == NULL) {
#ifdef _DEBUG_1_
	printf("FS Close Dir: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

	ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsCloseDir_remote", 1, 
		sizeof(*folder), folder);

	return *(int *)ans.return_val;
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    
    char *folderName = folder->name;
    char * tmpFolderName = malloc(strlen(folderName) + 1);
	strcpy(tmpFolderName, folderName);
	char *localFolderName = strtok(tmpFolderName, "/");
	if(localFolderName == NULL) {
		strcpy(localFolderName, folderName);
	}

#ifdef _DEBUG_1_
    printf("FS Read Dir: The foldername is %s\n", folderName);
#endif

    struct mounted_servers* rem_server = getRemoteServer(localFolderName);
	if( rem_server == NULL) {
#ifdef _DEBUG_1_
	printf("FS Read Dir: Could not fetch remote server belonging to folder name\n"); fflush(stdout);
#endif
		return NULL;
	}

	ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsReadDir_remote", 1, 
 			sizeof(*folder), folder);

    return (struct fsDirent *)(ans.return_val);
}

int fsOpen(const char *fname, int mode) {
	uint32_t clientIP = getPublicIPAddr();
	char * tmpFName = malloc(strlen(fname) + 1);
	strcpy(tmpFName, fname);
	char *localFolderName = strtok(tmpFName, "/");

	// should never be executed ideally, more of a safety net
	if(localFolderName == NULL) {
		strcpy(localFolderName, fname);
	}
#ifdef _DEBUG_1_
    printf("FS Open: The foldername is %s\n", localFolderName);
#endif


	struct mounted_servers *rem_server = getRemoteServer(localFolderName);
	if( rem_server == NULL) {
#ifdef _DEBUG_1_
	printf("FS Open: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

    ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsOpen_remote", 4,
    	sizeof(uint32_t), (void *)&clientIP,
    	strlen(localFolderName) + 1, localFolderName,
    	strlen(fname) + 1, fname,
    	sizeof(mode), (void *)&mode
    	);

    int fd = *(int *)ans.return_val;

    clientFD *newFD = malloc(sizeof(clientFD));
    newFD->fd = fd;
    newFD->ip = clientIP;
    newFD->next = clientFD_head;

    strcpy(newFD->localFolderName, localFolderName);

    clientFD_head = newFD;

    return fd;
}

struct clientFD * getClientByFD(const uint32_t ip, const int fd) {
	struct clientFD *tmp = clientFD_head;

	while(tmp != NULL) {
		if(tmp->ip == ip && tmp->fd == fd) {
			return tmp;
		}
		tmp = tmp->next;
	}
#ifdef _DEBUG_1_
	printf("Unable to find client on the server side with ip %d and fd %d\n", ip, fd); fflush(stdout);
#endif
	return NULL;
}

void freeClientFD(int fd) {
	clientFD * tmp = clientFD_head;
	clientFD * prev = NULL;
	for(; tmp != NULL; tmp = tmp->next) {
		if(tmp->fd == fd) {
			if(tmp == clientFD_head) {
				clientFD_head = tmp->next;
				free(tmp);
			}else {
				prev->next = tmp->next;
				free(tmp);
			}
		}
		prev = tmp;
	}
}

int fsClose(int fd) {
#ifdef _DEBUG_1_
	printf("FS Close:\n"); fflush(stdout);
#endif
	uint32_t clientIP = getPublicIPAddr();
	
	struct clientFD *cfd = getClientByFD(clientIP, fd);
	struct mounted_servers * ms;
	if(cfd != NULL) {
		ms = getRemoteServer(cfd->localFolderName);
	}
#ifdef _DEBUG_1_
	printf("MS: %s\n", ms->localFolderName); fflush(stdout);
#endif
	if( ms == NULL) {
#ifdef _DEBUG_1_
	printf("FS Open: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

	ans = make_remote_call(ms->srvIpOrDomName, ms->srvPort, "fsClose_remote", 3,
    	sizeof(uint32_t), (void *)&clientIP,
    	strlen(ms->localFolderName) + 1, ms->localFolderName,
    	sizeof(int), (void *)&fd
    	);

	int ret_val = *(int *)ans.return_val;

	if (ret_val == -1)
		return ret_val;

	freeClientFD(fd);

    return ret_val;
}

int fsRead(int fd, void *buf, const unsigned int count) {
#ifdef _DEBUG_1_
	printf("fsRead():\n"); fflush(stdout);
#endif
	uint32_t clientIP = getPublicIPAddr();

	struct clientFD *cfd = getClientByFD(clientIP, fd);
	struct mounted_servers *ms;
	if(cfd != NULL) {
		ms = getRemoteServer(cfd->localFolderName);
	}
#ifdef _DEBUG_1_
	printf("MS: %s\n", ms->localFolderName); fflush(stdout);
#endif
	if( ms == NULL) {
#ifdef _DEBUG_1_
	printf("FS Open: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

	ans = make_remote_call(ms->srvIpOrDomName, ms->srvPort, "fsRead_remote", 4,	
		sizeof(uint32_t), (void *)&clientIP,
		strlen(ms->localFolderName) + 1, ms->localFolderName,
		sizeof(int), (void *)&fd,
		sizeof(int), &count
		);

	char *serverBuf = (char *)ans.return_val;
	int numBytesRead = *(int *)serverBuf;
	serverBuf += sizeof(numBytesRead);
	int errNo = *(int *)serverBuf;
	serverBuf += sizeof(errNo);

	buf = serverBuf;
#ifdef _DEBUG_1_
	printf("num bytes read: %d, errNo %d and buf %s\n", numBytesRead, errNo, serverBuf);
#endif

	return numBytesRead;
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
#ifdef _DEBUG_1
	printf("fsWrite():\n"); fflush(stdout);
#endif
	uint32_t clientIP = getPublicIPAddr();

	struct clientFD *cfd = getClientByFD(clientIP, fd);
	struct mounted_servers *ms;
	if(cfd != NULL) {
		ms = getRemoteServer(cfd->localFolderName);
	}
#ifdef _DEBUG_1_
	printf("MS: %s\n", ms->localFolderName); fflush(stdout);
#endif
	if(ms == NULL) {
#ifdef _DEBUG_1_
	printf("FS Write: Could not fetch the remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

	ans = make_remote_call(ms->srvIpOrDomName, ms->srvPort, "fsWrite_remote", 4,
		sizeof(uint32_t), (void *)&clientIP,
		sizeof(int), (void *)&fd,
		sizeof(int), (void *)&count,
		strlen(buf) + 1, buf
		);
    
    char *serverBuf =(char *)ans.return_val;
    int numBytesWritten = *(int *)serverBuf;
    serverBuf += sizeof(numBytesWritten);
    int errNo = *(int *)serverBuf;

#ifdef _DEBUG_1_
    printf("num bytes written %d, errno %d\n", numBytesWritten, errNo);
#endif

    return numBytesWritten;
}

int fsRemove(const char *name) {
#ifdef _DEBUG_1_
	printf("fsRemove():\n"); fflush(stdout);
#endif

	uint32_t clientIP = getPublicIPAddr();

	char * tmpFName = malloc(strlen(name) + 1);
	strcpy(tmpFName, name);
	char *localFolderName = strtok(tmpFName, "/");

	// should never be executed ideally, more of a safety net
	if(localFolderName == NULL) {
		strcpy(localFolderName, name);
	}
#ifdef _DEBUG_1_
    printf("FS REmove: The foldername is %s\n", localFolderName);
#endif


	struct mounted_servers *ms = getRemoteServer(localFolderName);
	if( ms == NULL) {
#ifdef _DEBUG_1_
	printf("FS Remove: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
#endif
		return -1;
	}

	ans = make_remote_call(ms->srvIpOrDomName, ms->srvPort, "fsRemove_remote", 2,
		sizeof(uint32_t), (void *)&clientIP,
		strlen(name) + 1, (void *)name
		);

    char *serverBuf = (char *)ans.return_val;
    int retVal = *(int *)serverBuf;
    serverBuf += sizeof(retVal);
    int errNo = *(int *)serverBuf;

#ifdef _DEBUG_1_
    printf("FS Remove: errno %d return val %d\n", errNo, retVal);
#endif

    return retVal;
}
