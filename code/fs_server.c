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

FSDIR * fsOpenDirectory(const char * folderName) {
	//return (opendir(folderName));
	FSDIR fs_dir;
	fs_dir.dir = opendir(folderName);
	FSDIR *to_return = malloc(sizeof(fs_dir));

	to_return->dir = opendir(folderName);

	return to_return;
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
	char *tmpFolderName = malloc(strlen(folderName) + 1);
	strcpy(tmpFolderName, folderName);
	char *t = strtok(tmpFolderName, "/");
	int init_len = -1;
	int clientSideRootFolderSize = -1;

	if(t == NULL) {
		clientSideRootFolderSize = strlen(my_folder) + 1;
		init_len = strlen(folderName);
#ifdef _DEBUG_1_
	printf("Init len = %d\n", init_len);
#endif
	} else {
		clientSideRootFolderSize = strlen(my_folder);
		init_len = strlen(t);
		clientSideRootFolderSize += strlen(folderName) - init_len + 1;

#ifdef _DEBUG_1_
	printf("Init len = %d\n", init_len);
#endif
	}
	
#ifdef _DEBUG_1_
	printf("Client side root folder size %d\n", clientSideRootFolderSize);
#endif

	char *serverSideFolderPath = malloc(clientSideRootFolderSize);
	strcpy(serverSideFolderPath, my_folder);
	strcat(serverSideFolderPath, folderName + init_len);
	//strncat(serverSideFolderPath, '\0');

#ifdef _DEBUG_1_
	printf("Server side folder path %s and %d\n", serverSideFolderPath, strlen(serverSideFolderPath));
#endif

	// create local storage
	//struct client * cWho = getClient(clientWho, folderName);
	FSDIR *folderToOpen;// = malloc(sizeof(FSDIR));

#ifdef _DEBUG_1_
	printf("The size of FSDIR is %d\n", sizeof(FSDIR));
#endif

	folderToOpen = fsOpenDirectory(serverSideFolderPath);
	if(folderToOpen->dir == NULL) {
		printf("NULL FD\n");
	}
	
	//memcpy(&folderToOpen->who, cWho, sizeof(folderToOpen->who));
#ifdef _DEBUG_1_
	printf("Success in fsOpen directory\n");
#endif
	
#ifdef _DEBUG_1_
	printf("Read dir\n");
#endif
	//folderToOpenname = malloc(strlen(folderName) + 1);

	strcpy(folderToOpen->name, folderName);
#ifdef _DEBUG_1_
	printf("String copy\n");
#endif

	//strcpy(fDir->d_name, folderName);
	//fDir->d_type = DT_DIR; // folder
	
	//memcpy(&folderToOpen->entry, fDir, sizeof(*fDir));

#ifdef _DEBUG_1_
	//printf("Client info %s %d\n", folderToOpen->who.localFolderName, folderToOpen->who.clientIP);
	//printf("Dirent info %s\n", folderToOpen->entry.d_name);
#endif
	
#ifdef _DEBUG_1_
	//printf("fsOpenDir_remote: The directory opened is %s %s and entity Type %d\n", fDir->d_name, 
	//	folderToOpen->entry.d_name, fDir->d_type);
	printf("fsOpenDir_remote: The directory opened is %s\n", folderToOpen->name);
	printf("fsOpenDir_remote: The pointer is %p\n", folderToOpen);
#endif

	//folderToOpen = malloc(sizeof(tmpFolder));
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
    
    return to_return;
}


//return_type fsCloseDir_remote(const int nparams, arg_type* a) {
//#ifdef _DEBUG_1_
//	printf("FS Server: fsCloseDir\n"); fflush(stdout);
//#endif
//	if(nparams != 1) {
//#ifdef _DEBUG_1_
//	printf("fsCloseDir_remote: Number of params incorrect\n"); fflush(stdout);
//#endif
/*		errno = EINVAL;
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}
	FSDIR * curDir = (FSDIR *)a->arg_val;
	int *close_ret_int = (int *)malloc(sizeof(int));
	*close_ret_int = closedir(curDir);

	r.return_val = (void *)close_ret_int;
	r.return_size = sizeof(int);

	return r;
}
*/

int mount_folder(const char *folderName) {
	struct stat sbuf;

	return 0;
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
    //register_procedure("fsCloseDir_remote", 2, fsCloseDir_remote);
    //register_procedure("fsCloseDir_remote", 1, fsCloseDir_remote);

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}