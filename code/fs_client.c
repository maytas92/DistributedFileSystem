#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

int main(int argc, char *argv[]) {
    char *dirname = NULL;
    // TODO: Satyam please get rid of this.
    // For the reader, the point of this is to simple test mounting 
    // of two directories
    char *mount_test = "tmp2";

    if(argc < 4) {
        printf("usage: dirName serverIpOrName serverPort\n" );
    }
    if(argc > 1) dirname = argv[1];
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }

    printf("fsUnMount(): Unmounting %s with result %d\n", dirname, fsUnMount(dirname));
    printf("fsUnMount(): Unmouunting %s with result %d\n", mount_test, fsUnMount(mount_test));

    printf("fsMount(): Directory %s with result %d\n", dirname, fsMount(argv[2], atoi(argv[3]), dirname));
    printf("fsMount(): Directory %s with result %d\n", mount_test, fsMount(argv[2], atoi(argv[3]), mount_test));
    
    printf("fsUnMount(): Unmounting %s with result %d\n", dirname, fsUnMount(dirname));
    printf("fsUnMount(): Unmounting %s with result %d\n", mount_test, fsUnMount(mount_test));

    printf("fsMount(): Mounting %s with result %d\n", dirname, fsMount(argv[2], atoi(argv[3]), dirname));

    char * openDir = "tmp1";
    FSDIR *fd = fsOpenDir(openDir);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    }

    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    printf("Closing folder %s with return code %d\n", dirname, fsCloseDir(fd));

    int ff = fsOpen("tmp1/test.txt", 0);
    if(ff < 0) {
    perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);

    printf("fsClose(): %d\n", fsClose(ff));

    printf("fsUnMount(): UnMounting %s with result %d\n", dirname, fsUnMount(dirname));

    

    //printf("fsCloseDir(): %d\n", fsCloseDir(fd));

    /*
    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
    printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    if(errno != 0) {
    perror("fsReadDir");
    }

    printf("fsCloseDir(): %d\n", fsCloseDir(fd));

    int ff = fsOpen("/dev/urandom", 0);
    if(ff < 0) {
    perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);

    char fname[15];
    if(fsRead(ff, (void *)fname, 10) < 0) {
    perror("fsRead"); exit(1);
    }

    int i;
    for(i = 0; i < 10; i++) {
    //printf("%d\n", ((unsigned char)(fname[i]))%26);
    fname[i] = ((unsigned char)(fname[i]))%26 + 'a';
    }
    fname[10] = (char)0;
    printf("Filename to write: %s\n", (char *)fname);

    char buf[256];
    if(fsRead(ff, (void *)buf, 256) < 0) {
    perror("fsRead(2)"); exit(1);
    }

    printBuf(buf, 256);

    printf("fsClose(): %d\n", fsClose(ff));

    ff = fsOpen(fname, 1);
    if(ff < 0) {
    perror("fsOpen(write)"); exit(1);
    }

    if(fsWrite(ff, buf, 256) < 256) {
    fprintf(stderr, "fsWrite() wrote fewer than 256\n");
    }

    if(fsClose(ff) < 0) {
    perror("fsClose"); exit(1);
    }

    printf("fsRemove(%s): %d\n", fname, fsRemove(fname));
    */
    return 0;
}
