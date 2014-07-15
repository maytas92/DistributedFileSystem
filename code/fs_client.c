#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ece454_fs.h"

int main(int argc, char *argv[]) {
    char *dirname = NULL;

    if(argc < 4) {
        printf("usage: dirName serverIpOrName serverPort\n" );
    }
    if(argc > 1) dirname = argv[1];
    else {
        dirname = (char *)calloc(strlen(".")+1, sizeof(char));
        strcpy(dirname, ".");
    }

    printf("fsUnMount(): Unmounting %s with result %d\n", dirname, fsUnMount(dirname));

    printf("fsMount(): Directory %s with result %d\n", dirname, fsMount(argv[2], atoi(argv[3]), dirname));
    
    printf("fsUnMount(): Unmounting %s with result %d\n", dirname, fsUnMount(dirname));

    printf("fsMount(): Mounting %s with result %d\n", dirname, fsMount(argv[2], atoi(argv[3]), dirname));

    char *toOpenDir = malloc(strlen(dirname) + 1 + strlen(".") + 1);
    strcpy(toOpenDir, dirname);
    strcat(toOpenDir, "/");
    strcat(toOpenDir, ".");
    printf("Opening dir %s\n", toOpenDir);

    FSDIR *fd = fsOpenDir(toOpenDir);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    }

    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    printf("Closing folder %s with return code %d\n", dirname, fsCloseDir(fd));

    char *toOpen = malloc(strlen(dirname) + 1 + strlen("client_api.c") + 1);
    strcpy(toOpen, dirname);
    strcat(toOpen, "/");
    strcat(toOpen, "client_api.c");
    printf("opening file %s\n", toOpen);

    int ff = fsOpen(toOpen, 0);
    if(ff < 0) {
        perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);

    char fname[15];
    if(fsRead(ff, (void *)fname, 10) < 0) {
        perror("fsRead"); exit(1);
    }

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
