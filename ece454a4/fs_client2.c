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

    /*char *toOpenDir2 = malloc(strlen(dirname) + 1 + strlen("simplified_rpc") + 1);
    strcpy(toOpenDir2, dirname);
    strcat(toOpenDir2, "/");
    strcat(toOpenDir2, "simplified_rpc");
    printf("opening dir %s\n", toOpenDir2);
    */

    FSDIR *fd = fsOpenDir(toOpenDir);
    if(fd == NULL) {
        perror("fsOpenDir"); exit(1);
    }

    /*FSDIR *fd2 = fsOpenDir(toOpenDir2);
    if(fd2 == NULL) {
        perror("fsOpenDir"); exit(1);
    }

    printf("Reading dir %s\n", toOpenDir);
    struct fsDirent *fdent = NULL;
    for(fdent = fsReadDir(fd); fdent != NULL; fdent = fsReadDir(fd)) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }

    printf("Reading sub - dir %s\n", toOpenDir2);
    for(fdent = fsReadDir(fd2); fdent != NULL; fdent = fsReadDir(fd2)) {
        printf("\t %s, %d\n", fdent->entName, (int)(fdent->entType));
    }
    */
    char *toOpen = malloc(strlen(dirname) + 1 + strlen("test.txt") + 1);
    strcpy(toOpen, dirname);
    strcat(toOpen, "/");
    strcat(toOpen, "test.txt");
    printf("opening file test.txt in read mode %s\n", toOpen);

    int ff = fsOpen(toOpen, 1);
    if(ff < 0) {
        perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);

    usleep(10000000);
    /*
    char fname[15];
    if(fsRead(ff, (void *)fname, 10) < 0) {
        perror("fsRead"); exit(1);
    }

    printf("fsClose(): %d\n", fsClose(ff));
    
    ff = fsOpen(toOpen, 1);
    if(ff < 0) {
        perror("fsOpen"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff);
    */
    char *buf = "abcd";
    if(fsWrite(ff, buf, strlen(buf)) < strlen(buf)) {
        fprintf(stderr, "fsWrite() wrote fewer than 4\n");
    }

    printf("fsClose(): %s %d\n", toOpen, fsClose(ff));
    
    /*
    char *toOpen2 = malloc(strlen(dirname) + 1 + strlen("test2.txt") + 1);
    strcpy(toOpen2, dirname);
    strcat(toOpen2, "/");
    strcat(toOpen2, "test2.txt");
    printf("opening file in write mode %s\n", toOpen2);

    int ff2 = fsOpen(toOpen2, 1);
    if(ff2 < 0) {
        perror("fsOpen()"); exit(1);
    }
    else printf("fsOpen(): %d\n", ff2);

    if(fsWrite(ff2, buf, strlen(buf)) < strlen(buf)) {
        fprintf(stderr, "fsWrite() wrote fewer than 3\n");
    }

    printf("fsClose(): %s %d\n", toOpen, fsClose(ff));

    printf("fsClose(): %s %d\n", toOpen2, fsClose(ff2));

    printf("fsRemove(%s): %d\n", toOpen, fsRemove(toOpen));

    //printf("fsRemove(%s): %d\n", toOpen2, fsRemove(toOpen2));

    printf("Closing folder %s with return code %d\n", toOpenDir, fsCloseDir(fd));
    printf("Closing folder %s with return code %d\n", toOpenDir2, fsCloseDir(fd2));
    //printf("Closing folder %s with return code %d\n", toOpenDir2, fsCloseDir(fd2));

    printf("fsUnMount(): UnMounting %s with result %d\n", dirname, fsUnMount(dirname));

    */

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
