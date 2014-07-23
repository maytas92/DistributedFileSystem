#include "simplified_rpc/ece454rpc_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "ece454_fs.h"
#include <stdint.h>

#if 1
#define _DEBUG_1_
#endif

return_type r;

extern printRegisteredProcedures();

// is the served folder by the server side
char *my_folder;

struct client * client_head = NULL;

struct fileOpenServerPath * open_server_path_head = NULL;

struct fsDirent dent;

/*
* Adds a client to the linked list of clients that are maintained on the 
* server side. A client is identified by its IP Address and the folderName
* used to mount this server on the client side.
* This is called when the server mounts() a client.
*/
int addClient(const uint32_t newIP, const char *folderName) {
	if(newIP == 0) {
		errno = EINVAL;
#ifdef _DEBUG_1_
	printf("Adding client: Client IP is zero. Invalid input!\n"); fflush(stdout);
#endif
		return -1;
	}
	struct client * new_client = (struct client *)malloc(sizeof(struct client));

	new_client->next = client_head;
	new_client->clientIP = newIP;
	new_client->fileOpenHead = NULL;

	new_client->localFolderName = (char *)malloc(sizeof(folderName));
	strcpy(new_client->localFolderName, folderName);

	client_head = new_client;
#ifdef _DEBUG_1_
	printf("Added client: %d %s\n", newIP, folderName);
#endif
	return 0;
}

/*
* Deallocates memory for a given input client from the client linked list
* structure maintained by the server side.
*/
void freeClient(struct client *tmp) {
	free(tmp->localFolderName);
	free(tmp);
}

/*
* Removes a client linked list node from the server side. This is called 
* when the server unMounts the client. 
*/
int removeClient(const uint32_t ip, const char *folderName) {
	struct client * tmp = client_head;
	struct client * prev = NULL;
	for(tmp = client_head; tmp != NULL; tmp = tmp->next, prev = tmp) {
		if(tmp->clientIP == ip && !strcmp(tmp->localFolderName, folderName)) {
			if(tmp == client_head) {
				client_head = client_head->next;
				freeClient(tmp);
				return 0;
			} else {
				prev->next = tmp->next;
				freeClient(tmp);
				return 0;
			}
		}
	}
#ifdef _DEBUG_1_
	printf("Removing client: FAILED\n"); fflush(stdout);
#endif
	return -1;
}

/*
* Returns the client node given the identifying client IP address
* and the local folder name used by the client to identify the 
* remote server. 
*/
struct client * getClient(const uint32_t ip, const char *folderName) {
#ifdef _DEBUG_1_
	printf("IP is %d and folderName is %s\n", ip, folderName); fflush(stdout);
#endif
	struct client *tmp;
	for(tmp = client_head; tmp != NULL; tmp = tmp->next) {
		if(tmp->clientIP == ip && !strcmp(tmp->localFolderName, folderName)) {
			return tmp;
		}
	}
#ifdef _DEBUG_1_
	printf("Unable to find client on the server side with ip %d and folder name %s\n", ip, folderName); fflush(stdout);
#endif
	return NULL;
}
/*
* Builds the server side folder path given the client side folder path
* Note the folder path may be to subdirectory
*/
char * buildServerSideFolderPath(char *fname) {
	char *tmpFolderName = malloc(strlen(fname) + 1);
	strcpy(tmpFolderName, fname);
	char *t = strtok(tmpFolderName, "/");
	int init_len = -1;
	int clientSideRootFolderSize = -1;

	if(t == NULL) {
		clientSideRootFolderSize = strlen(my_folder) + 1;
		init_len = strlen(fname);
	} else {
		clientSideRootFolderSize = strlen(my_folder);
		init_len = strlen(t);
		clientSideRootFolderSize += strlen(fname) - init_len + 1;
	}

	char *serverSideFolderPath = malloc(clientSideRootFolderSize);
	strcpy(serverSideFolderPath, my_folder);
	strcat(serverSideFolderPath, fname + init_len);

	return serverSideFolderPath;
}

/*
* Opens a directory given the folder name as input.
* Returns a pointer to FSDIR as defined in fsOtherIncludes.h
*/
FSDIR * fsOpenDirectory(const char * folderName) {
	errno = 0;
	return (opendir(folderName));
}

/*
* Allows a client to mount this remote server locally.
*/
return_type fsMount_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsMount\n"); fflush(stdout);
#endif
	if(nparams != 2) {
		/* ERROR */
		// TODO: set perror
		// invalid argument
#ifdef _DEBUG_1_
	printf("fsMount_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		// set errno flag appropriately
		errno = EINVAL; 
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	char *localFolderName = (char *)a->arg_val;
#ifdef _DEBUG_1_
	printf("fsmount_remote: Local folder name: %s\n", localFolderName); fflush(stdout);
#endif
	
	uint32_t clientIP = *(uint32_t *)a->next->arg_val;
	int *ret_stat_ret = (int *)malloc(sizeof(int));
    *ret_stat_ret = 0;

	// To keep track of the <client, localFolderName> mapping on the server side
	if( addClient(clientIP, localFolderName) == -1) {
		*ret_stat_ret = -1;
	}

    r.return_val = (void *)ret_stat_ret;
    r.return_size = sizeof(int);

#ifdef _DEBUG_1_
    printf("FS Server: r.return_val %d\n", *(int *)r.return_val);
    printf("FS Server: r.return_size %d\n", r.return_size);
#endif
    
    return r;
}

/*
* Allows a client to unMount this remote server locally.
*/
return_type fsUnMount_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsUnMount\n"); fflush(stdout);
#endif

	char *localFolderName = (char *)a->arg_val;
#ifdef _DEBUG_1_
	printf("fsUnMount_remote: Local folder name: %s\n", localFolderName); fflush(stdout);
#endif

	uint32_t clientIP = *(uint32_t *)a->next->arg_val;
	int *ret_int = (int *)malloc(sizeof(int));
	*ret_int = 0;

	if( removeClient(clientIP, localFolderName) == -1) {
#ifdef _DEBUG_1_
	printf("FS Unmount: server side error\n"); fflush(stdout);
#endif
		*ret_int = -1;
	}
	
	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);
	return r;
}

/*
* Allows a client to open a directory or subdirectory under the 'root'
*/
return_type fsOpenDir_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsOpenDir\n"); fflush(stdout);
#endif
	if(nparams != 2) {
		/* ERROR */
		// TODO: set perror
		// invalid argument
#ifdef _DEBUG_1_
	printf("fsOpenDir_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		// set errno flag appropriately
		errno = EINVAL; 
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}
	// Get input parameters
	char *folderName = (char *)a->arg_val;
	uint32_t clientWho = *(uint32_t *)a->next->arg_val;

	// Build server side folder path
	char *serverSideFolderPath = buildServerSideFolderPath(folderName);

#ifdef _DEBUG_1_
	printf("Server side folder path %s and %d\n", serverSideFolderPath, strlen(serverSideFolderPath));
#endif

	// create local storage
	FSDIR *folderToOpen = malloc(sizeof(FSDIR));

#ifdef _DEBUG_1_
	printf("The size of FSDIR is %d\n", sizeof(FSDIR));
#endif

	folderToOpen->dir = fsOpenDirectory(serverSideFolderPath);
	folderToOpen->errNo = errno;
	if(folderToOpen->dir == NULL) {
		printf("NULL FD\n");
	}

	strcpy(folderToOpen->name, folderName);
	
#ifdef _DEBUG_1_
	printf("fsOpenDir_remote: The directory opened is %s\n", folderToOpen->name);
#endif

	r.return_val = (void *)folderToOpen;
	r.return_size = sizeof(*folderToOpen);

	return r;
}

/*
* Allows a client to read the contents of an input directory.
*/
return_type fsReadDir_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsReadDir\n"); fflush(stdout);
#endif
	if(nparams != 1) {
		/* ERROR */
		// TODO: set perror
		// invalid argument
#ifdef _DEBUG_1_
	printf("fsReadDir_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		// set errno flag appropriately
		errno = EINVAL; 
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	FSDIR *fDir = (FSDIR *)a->arg_val;

	struct fsDirent *curRead = fsReadDir(fDir);
	if(curRead == NULL) {
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	r.return_val = (void *)curRead;
	r.return_size = sizeof(*curRead);

	return r;
}

/*
* Returns a directory entry structure corresponding 
* to a readdir on the input FSDIR * folder.
*/
struct fsDirent *fsReadDir(FSDIR *folder) {
    const int initErrno = errno;
    errno = 0;
    struct dirent *d = readdir(folder->dir);

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

    struct fsDirent *to_return = malloc(sizeof(dent));
    to_return->entType = dent.entType;
    memcpy(&(to_return->entName), &(d->d_name), 256);

    return to_return;
}

/*
* Allows a client to close a directory. Presumably, the client 
* has called fsOpenDir_remote() prior to this.
*/
return_type fsCloseDir_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsCloseDir\n"); fflush(stdout);
#endif
	if(nparams != 1) {
#ifdef _DEBUG_1_
	printf("fsCloseDir_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		errno = EINVAL;
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}
	FSDIR * curDir = (FSDIR *)a->arg_val;

	int errNo;
	int retVal;
	int serverBufSize = sizeof(errNo) + sizeof(retVal);

	char *serverBuf = malloc(serverBufSize);
	char *tmpServerBuf = serverBuf;

	errno = 0;
	retVal = closedir(curDir->dir);
	*(int *)tmpServerBuf = retVal;
	tmpServerBuf += sizeof(retVal);
	// set error code
	*(int *)tmpServerBuf = errno;

	r.return_val = (void *)serverBuf;
	r.return_size = serverBufSize;

	return r;
}

/*
* Allows a client to open a file on the remote server.
*/
return_type fsOpen_remote(const int nparams, arg_type * a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsOpen\n"); fflush(stdout);
#endif	
	if(nparams != 4) {
#ifdef _DEBUG_1_
	printf("fsOpen_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		errno = EINVAL;
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	// get client side parameters
	uint32_t clientIP = *(uint32_t *)a->arg_val;
	char *localFolderName = (char *)a->next->arg_val;
	char *fname = (char *)a->next->next->arg_val;
	int mode = *(int *)a->next->next->next->arg_val;

	// Server side path building 
	char *serverSideFolderPath = buildServerSideFolderPath(fname);

#ifdef _DEBUG_1_
	printf("Server side folder path %s and %d\n", serverSideFolderPath, strlen(serverSideFolderPath));
#endif

#ifdef _DEBUG_1_
	printf("FS open: Client side parameters set up %d %s %s %d\n", clientIP, localFolderName, fname, mode); fflush(stdout);
#endif

	struct client * curClient = getClient(clientIP, localFolderName);

	int errNo;
	int retVal;
	int randomRetVal;
	int serverBufSize = sizeof(errNo) + sizeof(retVal);

	char *serverBuf = malloc(serverBufSize);
	char *tmpServerBuf = serverBuf;

	// look through all open files for the client
	fileOpen * tmp = curClient->fileOpenHead;

	/* This is to check if the SAME client tries to open the same 
	*  file name twice we return -1.  
	*/
	while(tmp != NULL) {
		if(!strcmp(tmp->name, fname)) {
			// file already open. TODO: check
			printf("File already open on the server side\n");
			*(int *)tmpServerBuf = -1;
			tmpServerBuf += sizeof(retVal);
			// set error code
			*(int *)tmpServerBuf = -1;

			r.return_val = (void *)serverBuf;
			r.return_size = serverBufSize;

			return r;
		}
		tmp = tmp->next;
	}

	/* This is to check if a different client tries to open the same
	*  file name that is already open from another client.
	*/
	fileOpenServerPath *tmpServerPath = open_server_path_head;
	while(tmpServerPath != NULL) {
		if(!strcmp(tmpServerPath->name, serverSideFolderPath)) {
			printf("File already open by another client\n");
			// TODO: Add some wait some message to inform the client
			*(int *)tmpServerBuf = -100;
			tmpServerBuf += sizeof(retVal);
			// set error code
			*(int *)tmpServerBuf = errno;

			r.return_val = (void *)serverBuf;
			r.return_size = serverBufSize;

			return r;
		}
		tmpServerPath = tmpServerPath->next;
	}

	// client wishes to write to the file
	if(mode == 1) {
		// remove the file so that it can be created and overwritten
		fsRemove(serverSideFolderPath);
	}
	// else
	fileOpen * newFile = malloc(sizeof(fileOpen));
	newFile->next = curClient->fileOpenHead;
	newFile->mode = mode;
	strncpy(newFile->name, fname, 256);

	curClient->fileOpenHead = newFile;
	
	retVal = fsOpen(serverSideFolderPath, mode);

	// If the client was able to successfully open this file
	// then add this open file to a linked list
	// so that in the future we do not allow re-opening
	// of this file by another client.
	if(retVal > 0) {
		/*
		* To keep track of open files on the server from ALL clients
		*/
		printf("Got here! This means that the server file path was added %s\n", serverSideFolderPath);

		fileOpenServerPath *new_server_file_path = malloc(sizeof(fileOpenServerPath));
		strcpy(new_server_file_path->name, serverSideFolderPath);
		new_server_file_path->fd = retVal;
		
		new_server_file_path->next = open_server_path_head;
		open_server_path_head = new_server_file_path;
	}
	randomRetVal = rand();
	*(int *)tmpServerBuf = randomRetVal;
	tmpServerBuf += sizeof(randomRetVal);
	// set error code
	*(int *)tmpServerBuf = errno;
	newFile->fd = retVal;
	if(retVal > 0) {
		newFile->randomFd = randomRetVal;
	} else {
		newFile->randomFd = -1;
	}
#ifdef _DEBUG_1_
	printf("FS OPEN SERVER(): ret_val %d errno %d and ret_size %d\n", randomRetVal, errno, serverBufSize);
#endif
	r.return_val = (void *)serverBuf;
	r.return_size = serverBufSize;

	return r;
}

/*
* Simple function to open a file given the filename 
* and the mode.
*/
int fsOpen(const char *fname, int mode) {
	errno = 0;
    int flags = -1;

    if(mode == 0) {
		flags = O_RDONLY;
    }
    else if(mode == 1) {
		flags = O_WRONLY | O_CREAT;
    }

    return(open(fname, flags, S_IRWXU));
}

/*
* When a client calls fsClose. Then deallocate memory for the 
* client node structure that maintains a linked list of 
* clients that have the file open on the server side.
*/
void freeOpenClient(int fd, struct client *curClient) {
	fileOpen *prev = NULL;
	fileOpen *tmp = curClient->fileOpenHead;

	for(; tmp != NULL; tmp = tmp->next) {
		if(tmp->fd == fd) {
			if(tmp == curClient->fileOpenHead) {
				curClient->fileOpenHead = tmp->next;
				free(tmp);
				printf("head"); fflush(stdout);
			} else {
				prev->next = tmp->next;
				free(tmp);
				printf("non head"); fflush(stdout);
			}
			break;
		}
		printf("update"); fflush(stdout);
		prev = tmp;

	}
	printf("Blah"); fflush(stdout);
}

/* 
* Allows a client to close a file on this remote server.
*/
return_type fsClose_remote(const int nparams, arg_type *a) {
#ifdef _DEBUG_1_
	printf("FS Close remote:\n"); fflush(stdout);
#endif
	if(nparams != 3) {
#ifdef _DEBUG_1_
	printf("FS Close remote: Number of params incorrect\n"); fflush(stdout);
#endif
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	uint32_t clientIP = *(uint32_t *)a->arg_val;
	char *localFolderName = (char *)a->next->arg_val;
	int rfd = *(int *)a->next->next->arg_val;
	int fd;
	struct client * curClient = getClient(clientIP, localFolderName);

	// look through all open files for the client
	fileOpen * tmp = curClient->fileOpenHead;

	/* This is to check if the SAME client tries to open the same 
	*  file name twice we return -1.  
	*/
	while(tmp != NULL) {
		if(tmp->randomFd == rfd) {
			// file already open. TODO: check
			printf("File found with random fd %d\n", rfd);
			fd = tmp->fd;
			printf("File found with fd %d\n", fd);
		}
		tmp = tmp->next;
	}

#ifdef _DEBUG_1_
	printf("start iterating %d\n", fd); fflush(stdout);
#endif

	freeOpenClient(fd, curClient);

	// allow other clients that are blocked to open this file
	fileOpenServerPath *tmpServerPath = open_server_path_head;
	fileOpenServerPath *prevServerPath = NULL;
	while(tmpServerPath != NULL) {
		if(tmpServerPath->fd == fd) {
			// found the server file path that is being closed
			if(tmpServerPath == open_server_path_head) {
				open_server_path_head = tmpServerPath->next;
			} else {
				prevServerPath->next = tmpServerPath->next;
			}
			free(tmpServerPath);
			break;
		}
		prevServerPath = tmpServerPath;
	}

	int errNo;
	int retVal;
	int serverBufSize = sizeof(errNo) + sizeof(retVal);

	char *serverBuf = malloc(serverBufSize);
	char *tmpServerBuf = serverBuf;
	
	retVal = fsClose(fd);
	*(int *)tmpServerBuf = retVal;
	tmpServerBuf += sizeof(retVal);
	// set error code
	*(int *)tmpServerBuf = errno;

	r.return_val = (void *)serverBuf;
	r.return_size = serverBufSize;

	return r;
}

/*
* Simple function that closes a file given a file descriptor that was
* presumably returned from fsOpen() on the server side.
*/
int fsClose(int fd) {
	errno = 0;
    return(close(fd));
}

/*
* Allows a client to read the contents of a file on the remote server.
*/
return_type fsRead_remote(const int nparams, arg_type *a) {
#ifdef _DEBUG_1_
	printf("fsRead_remote: ()\n"); fflush(stdout);
#endif
	if(nparams != 4) {
#ifdef _DEBUG_1_
	printf("fsRead_remote: Number of parameters incorret\n"); fflush(stdout);
#endif
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	uint32_t clientIP = *(uint32_t *)a->arg_val;
	a = a->next;
	char *localFolderName = (char *)a->arg_val;
	a = a->next;
	int rfd = *(int *)a->arg_val;
	a = a->next;
	int count = *(int *)a->arg_val;
	int fd;

	struct client * curClient = getClient(clientIP, localFolderName);

	// look through all open files for the client
	fileOpen * tmp = curClient->fileOpenHead;

	/* This is to check if the SAME client tries to open the same 
	*  file name twice we return -1.  
	*/
	while(tmp != NULL) {
		if(tmp->randomFd == rfd) {
			// file already open. TODO: check
			printf("File found with random fd %d\n", rfd);
			fd = tmp->fd;
			printf("File found with fd %d\n", fd);
		}
		tmp = tmp->next;
	}

	int errCode;
	int numBytesRead;
	int bufMetaDataSize = sizeof(numBytesRead) + sizeof(errCode);
	int bufSize =  bufMetaDataSize + count;

	char *buf = malloc(bufSize);
	char *tmpBuf = buf;

	numBytesRead = fsRead(fd, buf + bufMetaDataSize, count);
	*(int *)tmpBuf = numBytesRead;
	tmpBuf += sizeof(int);
	*((int *)tmpBuf) = errno;
	tmpBuf += sizeof(int);

	tmpBuf += numBytesRead;
	*(char *)tmpBuf = '\0';

	r.return_val = (void *)(buf);
	r.return_size = bufSize;

#ifdef _DEBUG_1_
	printf("In read FD %d\n", fd);
	printf("Return size: %d\n", r.return_size); fflush(stdout);
	printf("Return val is %s\n", buf); fflush(stdout);
#endif

	return r;
}

/*
* Simple function to read a file given a file descriptor, input buffer to
* store the data and the number of bytes to read.
*/
int fsRead(int fd, void *buf, const unsigned int count) {
	errno = 0;
    return(read(fd, buf, (size_t)count));
}

/*
* Allows a client to write to a file on the remote server.
*/
return_type fsWrite_remote(const int nparams, arg_type *a) {
#ifdef _DEBUG_1_
	printf("fsWrite_remote: ()\n"); fflush(stdout);
#endif
	if(nparams != 5) {
#ifdef _DEBUG_1_
	printf("fsWrite_remote: Number of parameters incorrect\n"); fflush(stdout);
#endif
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}	

	uint32_t clientIP = *(uint32_t *)a->arg_val;
	a = a->next;
	char *localFolderName = (char *)a->arg_val;
	a = a->next;
	int rfd = *(int *)a->arg_val;
	a = a->next;
	int count = *(int *)a->arg_val;
	a = a->next;
	char *bufToWrite = (char *)a->arg_val;

	int numBytesWritten;
	int errNo;
	int serverBufSize = sizeof(numBytesWritten) + sizeof(errNo);
	char *serverBuf = malloc(serverBufSize);
	char *tmpServerBuf = serverBuf;
	int fd;

	struct client * curClient = getClient(clientIP, localFolderName);

	// look through all open files for the client
	fileOpen * tmp = curClient->fileOpenHead;

	/* This is to check if the SAME client tries to open the same 
	*  file name twice we return -1.  
	*/
	while(tmp != NULL) {
		if(tmp->randomFd == rfd) {
			// file already open. TODO: check
			printf("File found with random fd %d\n", rfd); fflush(stdout);
			fd = tmp->fd;
			printf("File found with fd %d\n", fd); fflush(stdout);
		}
		tmp = tmp->next;
	}

	// to check if fd is open first
	// TODO
	printf("FD is %d, buftowrite is %s and count is %d\n", fd, bufToWrite, count);

	numBytesWritten = fsWrite(fd, bufToWrite, count);
#ifdef _DEBUG_1_
	printf("Num bytes written %d\n", numBytesWritten);
#endif
	*(int *)tmpServerBuf = numBytesWritten;

	tmpServerBuf += sizeof(numBytesWritten);
	*(int *)tmpServerBuf = errno;
#ifdef _DEBUG_1_
	printf("Err no is %d\n", errno);
#endif
	
#ifdef _DEBUG_1_
	printf("return val %s and return size %d\n", serverBuf, serverBufSize);
#endif
	r.return_val = (void *)serverBuf;
	r.return_size = serverBufSize;

	return r;
}

/*
* Simple function to write to file mapping to 'fd' with the contents of 
* 'buf' and number of bytes = count.
*/
int fsWrite(int fd, const void *buf, const unsigned int count) {
	errno = 0;
    return(write(fd, buf, (size_t)count)); 
}

/*
* Simple function to remove a file from the remote server.
*/
return_type fsRemove_remote(const int nparams, arg_type *a) {
#ifdef _DEBUG_1_
	printf("fsRemove_remote: ()\n"); fflush(stdout);
#endif
	if(nparams != 2) {
#ifdef _DEBUG_1_
	printf("fsRemove_remote: Number of parameters incorret\n"); fflush(stdout);
#endif
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}	

	uint32_t clientIP = a->arg_val;
	a = a->next;
	char *dirName = a->arg_val;
	char *serverDirName = buildServerSideFolderPath(dirName);
	
	int errNo;
	int retVal;
	int serverBufSize = sizeof(errNo) + sizeof(retVal);

	char *serverBuf = malloc(serverBufSize);
	char *tmpServerBuf = serverBuf;
	
	retVal = fsRemove(serverDirName);	
#ifdef _DEBUG_1_
	printf("return value in fs remove () is %d\n", retVal);
#endif
	*(int *)tmpServerBuf = retVal;
	tmpServerBuf += sizeof(retVal);

	r.return_val = (void *)serverBuf;
	r.return_size = serverBufSize;

	return r;
}

/*
* Simple function to remove a file from the server side.
*/
int fsRemove(const char *name) {
	errno = 0;
    return(remove(name));
}

/*
* MAIN Function on the server side. 
* Inputs: directory to be served.
* Registers the necessary procedures that may be called from 
* the client_api.c
* Launches the TCP/IP server.
*/
int main(int argc, char *argv[]) {
	srand(time(NULL));
	if(argc < 2) {
		printf("usage: servedFolder\n");
		return -1;
	}
	my_folder = argv[1];
    register_procedure("fsMount_remote", 2, fsMount_remote);
    register_procedure("fsUnMount_remote", 2, fsUnMount_remote);
    register_procedure("fsOpenDir_remote", 2, fsOpenDir_remote);
    register_procedure("fsReadDir_remote", 1, fsReadDir_remote);
    register_procedure("fsCloseDir_remote", 1, fsCloseDir_remote);
    register_procedure("fsOpen_remote", 4, fsOpen_remote);
    register_procedure("fsClose_remote", 3, fsClose_remote);
    register_procedure("fsRead_remote", 4, fsRead_remote);
    register_procedure("fsWrite_remote", 5, fsWrite_remote);
    register_procedure("fsRemove_remote", 2, fsRemove_remote);

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}