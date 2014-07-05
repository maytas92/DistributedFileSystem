#include "simplified_rpc/ece454rpc_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ece454_fs.h"

#if 1
#define _DEBUG_1_
#endif

return_type r;

extern printRegisteredProcedures();

return_type fsMount_remote(const int nparams, arg_type* a) {

	if(nparams != 1) {
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
	
    int *ret_stat_ret = (int *)malloc(sizeof(int));
    *ret_stat_ret = mount_folder(localFolderName);

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
	int *ret_int = (int *)malloc(sizeof(int));
	*ret_int = 0;
	r.return_val = (void *)ret_int;
	r.return_size = sizeof(int);
	return r;
}

int mount_folder(const char *folderName) {
	struct stat sbuf;

	return stat(folderName, &sbuf);
}


int main() {
    register_procedure("fsMount_remote", 1, fsMount_remote);
    register_procedure("fsUnMount_remote", 1, fsUnMount_remote);
    //register_procedure("fsOpenDir_remote", 1, fsOpenDir_remote);
    //register_procedure("fsCloseDir_remote", 1, fsCloseDir_remote);

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}