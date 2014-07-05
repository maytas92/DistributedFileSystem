/*
 * ECE 454: Distributed Systems. 
 * University of Waterloo
 * Khushi Sohi
 * Satyam Gupta
 * CLIENT API for the Distributed File System
*/
#include "ece454_fs.h"
#include <string.h>
#include "simplified_rpc/ece454rpc_types.h"

#if 1
#define _DEBUG_1_
#endif

 struct fsDirent dent;

 /* 
* Linked list of servers that have been mounted on a client
*/
struct mounted_servers {
	char * srvIpOrDomName;
	int srvPort;
	char * localFolderName;
	struct mounted_servers *next;
};

// Keeps track of the current number of mounted remote servers
// Note even if the same server is mounted twice with a different
// 'myFolder' then this counter would be incremented
static int num_mounted_servers = 0;

struct mounted_servers * ms_head = NULL;

return_type ans;

void freeMountedServer(struct mounted_servers *to_free) {
	free(to_free->srvIpOrDomName);
	free(to_free->localFolderName);
	free(to_free);
}

void addMountedServer(const char * srvIpOrDomName, int srvPort, const char * localFolderName) {
	struct mounted_servers *ms = (struct mounted_servers *)malloc(sizeof(struct mounted_servers));
	
	ms->next = ms_head;
	
	ms->srvIpOrDomName = malloc(strlen(srvIpOrDomName) + 1);
	strcpy(ms->srvIpOrDomName, srvIpOrDomName);

#ifdef _DEBUG_1_
	printf("The mounted server name is %s\n", ms->srvIpOrDomName);
#endif
	ms->srvPort = srvPort;

#ifdef _DEBUG_1_
	printf("The mounted server port is %d\n", ms->srvPort);
#endif
	
	ms->localFolderName = malloc(strlen(localFolderName) + 1);
	strcpy(ms->localFolderName, localFolderName);

#ifdef _DEBUG_1_
	printf("The mounted local folder name is %s\n", ms->localFolderName);
#endif
	ms_head = ms;
	
	num_mounted_servers++;
#ifdef _DEBUG_1_
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
#ifdef _DEBUG_1_
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
#ifdef _DEBUG_1_
	printf("fsUnMount: tmp->localFolderName %s\n", tmp->localFolderName);
	printf("fsUnMount: localFolderName Input %s\n", localFolderName);
#endif
		if( !strcmp(tmp->localFolderName, localFolderName) ) {
			num_mounted_servers--;
			prev->next = tmp->next;
			// TODO: Into a function
			freeMountedServer(tmp);
#ifdef _DEBUG_1_
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

	printf("fsMount:\n"); fflush(stdout);

#ifdef _DEBUG_1_
	printf("fsMount Client: %s\n", localFolderName); fflush(stdout);
#endif

	// Begin Client side storage of mounted servers
	addMountedServer(srvIpOrDomName, srvPort, localFolderName);
 	// End of client side storage

 	ans = make_remote_call(srvIpOrDomName, srvPort, "fsMount_remote", 1, 
 			sizeof(localFolderName), localFolderName );

#ifdef _DEBUG_1_
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
	return removeMountedServer(localFolderName);
}

int fsCloseDir(FSDIR *folder) {
    return(closedir(folder));
}

struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    struct dirent *d = readdir(folder);

    if(d == NULL) {
	if(errno == initErrno) errno = 0;
	return NULL;
    }

    if(d->d_type == DT_DIR) {
	dent.entType = 1;
    }
    else if(d->d_type == DT_REG) {
	dent.entType = 0;
    }
    else {
	dent.entType = -1;
    }

    memcpy(&(dent.entName), &(d->d_name), 256);
    return &dent;
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
