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

#if 1
#define _DEBUG_2_
#endif

struct fsDirent dent;

// Keeps track of the current number of mounted remote servers
// Note even if the same server is mounted twice with a different
// 'myFolder' then this counter would be incremented
static int num_mounted_servers = 0;

struct mounted_servers * ms_head = NULL;

return_type ans;

struct mounted_servers * getRemoteServer(const char * folderName) {
	// base check
	if(ms_head == NULL) {
		return NULL;
	}
	struct mounted_servers *tmp;
	for(tmp = ms_head; tmp != NULL; tmp = tmp->next) {
		if( !strcmp(tmp->localFolderName, folderName) ) {
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
	//printf("The mounted server length is %d which should match %d or %d\n", strlen(srvIpOrDomName) + 1, 
	//	sizeof(ms->srvIpOrDomName), strlen(ms->srvIpOrDomName) + 1);
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
#ifdef _DEBUG_2_
	printf("fsUnMount: tmp->localFolderName %s\n", tmp->localFolderName);
	printf("fsUnMount: localFolderName Input %s\n", localFolderName);
#endif
		if( !strcmp(tmp->localFolderName, localFolderName) ) {
			num_mounted_servers--;
			prev->next = tmp->next;
			// TODO: Into a function
			freeMountedServer(tmp);
#ifdef _DEBUG_2_
	printf("fsUnMount: Unmounting %s\n", localFolderName); fflush(stdout);
#endif			
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
#ifdef _DEBUG_1_
	//printf("Size of rem_server->localFolderName is %d\n", sizeof(rem_server->localFolderName));
	//printf("And its value is %s\n", rem_server->localFolderName);
#endif
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

//int fsCloseDir(FSDIR *folder) {
//#ifdef _DEBUG_1_
//	printf("FS Close Dir:\n"); fflush(stdout);
//#endif
	// http://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html
//	char *folderName = folder->d_name;
//	struct mounted_servers *rem_server= getRemoteServer(folderName);
//	if( rem_server == NULL) {
//#ifdef _DEBUG_1_
//	printf("FS Close Dir: Count not fetch remote server belonging to FSDIR * folder\n"); fflush(stdout);
//#endif
//		return NULL:
//	}
	//FSDIR *curFolder = (FSDIR *)malloc(sizeof(folder));
	//curFolder->d_name = folderName;
	//curFolder->d_ino = folder->d_ino;
//	ans = make_remote_call(rem_server->srvIpOrDomName, rem_server->srvPort, "fsCloseDir_remote", 1, 
//		sizeof(curFolder), curFolder);

//	return *(int *)ans.return_val;
//}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    
    char *folderName = folder->name;
    //struct dirent *d = readdir(folder);
    //char *folderName = d->d_name;
#ifdef _DEBUG_1_
    printf("FS Read Dir: The foldername is %s\n", folderName);
#endif

    struct mounted_servers* rem_server = getRemoteServer(folderName);
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
    int flags = -1;

    if(mode == 0) {
	flags = O_RDONLY;
    }
    else if(mode == 1) {
	flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));
}

int fsClose(int fd) {
    return(close(fd));
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return(read(fd, buf, (size_t)count));
}

int fsWrite(int fd, const void *buf, const unsigned int count) {
    return(write(fd, buf, (size_t)count)); 
}

int fsRemove(const char *name) {
    return(remove(name));
}
