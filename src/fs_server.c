#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"
#include "ece454_fs.h"
#include <errno.h>

#if 1
#define _DEBUG_1_
#endif

extern printRegisteredProcedures();

return_type r;

return_type fsMount_remote(const int nparams, arg_type* a) {

	if(nparams != 1) {
		/* ERROR */
		// TODO: set perror
		// invalid argument
#ifdef _DEBUG_1_
	printf("fsMount_remote: Number of params incorrect\n"); fflush(stdout);
#endif
		errno = EINVAL; 
		r.return_size = 0;
		r.return_val = NULL;
		return r;
	}

	char *localFolderName = (char *)a->arg_val;
#ifdef _DEBUG_1_
	printf("fsmount_remote: Local folder name: %s\n", localFolderName); fflush(stdout);
#endif
	struct stat sbuf;

	int stat_ret = stat(localFolderName, &sbuf);
#ifdef _DEBUG_1_
    printf("FS Server: fsMount\n"); fflush(stdout);
#endif
    int *ret_int = (int *)malloc(sizeof(int));
    *ret_int = stat_ret;
    r.return_val = (void *)ret_int;
    r.return_size = sizeof(int);

    
    return r;
}

return_type fsUnMount_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1_
	printf("FS Server: fsUnMount\n"); fflush(stdout);
#endif
	int *ret_int = (int *)malloc(sizeof(int));
	*ret_int = 0;
	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);
	return r;
}

return_type fsOpenDir_remote(const int nparams, arg_type* a) {
#ifdef _DEBUG_1
	printf("FS Server: fsOpenDir\n"); fflush(stdout);
#endif
	return r;
}

int main() {
    register_procedure("fsMount_remote", 1, fsMount_remote);
    register_procedure("fsUnMount_remote", 1, fsUnMount_remote);
    register_procedure("fsOpenDir_remote", 1, fsOpenDir_remote);

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}
