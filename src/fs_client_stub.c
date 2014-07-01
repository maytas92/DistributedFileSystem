/*
 * ECE 454: Distributed Systems. 
 * University of Waterloo
 * Khushi Sohi
 * Satyam Gupta
 * CLIENT API for the Distributed File System
*/

#include <stdio.h>
#include <string.h>
#include "ece454rpc_types.h"
#include "ece454_fs.h"

struct fsDirent dent;

extern void recvbytes(int, void *, ssize_t);
extern void sendbytes(int, void *, ssize_t);

return_type ans;
/*
* Mounts the remote folder locally. 
* Returns 0 on success and -1 on failure. Errno is set appropriately.
* 
*/
int fsMount(const char *srvIpOrDomName, const unsigned int srvPort, const char *localFolderName) {
	printf("fsMount:\n"); fflush(stdout);
	return_type ans;
 	ans = make_remote_call(srvIpOrDomName, srvPort, "fsMount", 0 );
 	printf("Return val: ", ans.return_val);
 	struct stat sbuf;

    return(stat(localFolderName, &sbuf));
 	//return 0;
}

/*
* Unmounts a remote file system. 
* Returns 0 on success, -1 on failure. Errno is set appropriately.
*/
int fsUnMount(const char *localFolderName) {
	return 0;
}

FSDIR* fsOpenDir(const char *folderName) {
    return(opendir(folderName));
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
