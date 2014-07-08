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

struct fsDirent dent;

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

void freeClient(struct client *tmp) {
	free(tmp->localFolderName);
	free(tmp);
}

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

FSDIR * fsOpenDirectory(const char * folderName) {
	return (opendir(folderName));
}


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
		// TODO: set errno flag
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	r.return_val = (void *)curRead;
	r.return_size = sizeof(*curRead);

	return r;
}

struct fsDirent *fsReadDir(FSDIR *folder) {
#ifdef _DEBUG_1_
	printf("Here\n"); fflush(stdout);
#endif
    const int initErrno = errno;
    struct dirent *d = readdir(folder->dir);
#ifdef _DEBUG_1_
    printf("read\n"); fflush(stdout);
#endif
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
    //return &dent;
    return to_return;
}


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
	int *close_ret_int = (int *)malloc(sizeof(int));
	*close_ret_int = closedir(curDir->dir);

	r.return_val = (void *)close_ret_int;
	r.return_size = sizeof(int);

	return r;
}


int mount_folder(const char *folderName) {
	struct stat sbuf;

	return 0;
}
// builds the server side folder path given the client side folder path
// Note the folder path may be to subdirectory



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

	int *ret_int = (int *)malloc(sizeof(int));
	// look through all open files for the client
	fileOpen * tmp = curClient->fileOpenHead;

	while(tmp != NULL) {
		if(!strcmp(tmp->name, fname)) {
			// file already open. TODO: check
			*ret_int = -1;
			r.return_size = (void *)ret_int;
			r.return_size = sizeof(int);

			return r;
		}
		tmp = tmp->next;
	}

	// else
	fileOpen * newFile = malloc(sizeof(fileOpen));
	newFile->next = curClient->fileOpenHead;
	newFile->mode = mode;
	strncpy(newFile->name, fname, 256);

	curClient->fileOpenHead = newFile;
	
	*ret_int = fsOpen(serverSideFolderPath, mode);
	newFile->fd = *ret_int;

#ifdef _DEBUG_1_
	printf("The return value is %d\n", *ret_int);
#endif
	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);

	return r;
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

void freeOpenClient(int fd, struct client *curClient) {
	fileOpen *prev = NULL;
	fileOpen *tmp = curClient->fileOpenHead;

	for(; tmp != NULL; tmp = tmp->next) {
		if(tmp->fd == fd) {
			if(tmp == curClient->fileOpenHead) {
				curClient->fileOpenHead = tmp->next;
				free(tmp);
			} else {
				prev->next = tmp->next;
				free(tmp);
			}
		}
		prev = tmp;
	}
}

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
	int fd = *(int *)a->next->next->arg_val;

	struct client * curClient = getClient(clientIP, localFolderName);

	int *ret_int = (int *)malloc(sizeof(int));

#ifdef _DEBUG_1_
	printf("start iterating\n"); fflush(stdout);
#endif

	freeOpenClient(fd, curClient);
	
	*ret_int = fsClose(fd);

	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);

	return r;
}

int fsClose(int fd) {
    return(close(fd));
}

return_type fsRead_remote(const int nparams, arg_type *a) {
#ifdef _DEBUG_1_
	printf("fsRead_remote: ()\n");
#endif
	if(nparams != 4) {
#ifdef _DEBUG_1_
	printf("fsRead_remote: Number of parameters incorret\n");
#endif
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	uint32_t clientIP = *(uint32_t *)a->arg_val;
	char *localFolderName = (char *)a->next->arg_val;
	int fd = *(int *)a->next->next->arg_val;
	int count = *(int *)a->next->next->next->arg_val;

	char *buf = malloc(count);

	int *ret_int = *(int *)malloc(sizeof(int));
	*ret_int = fsRead(fd, buf, count);

	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);

	return r;
}

int fsRead(int fd, void *buf, const unsigned int count) {
    return(read(fd, buf, (size_t)count));
}

int main(int argc, char *argv[]) {
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

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}