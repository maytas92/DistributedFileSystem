#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ece454rpc_types.h"

#if 1
#define _DEBUG_1_
#endif

extern printRegisteredProcedures();

return_type r;

return_type fsMount() {
    printf("Sample server app. fsMount\n"); fflush(stdout);

    int *ret_int = (int *)malloc(sizeof(int));
    *ret_int = 0;
    r.return_val = (void *)ret_int;
    r.return_size = sizeof(int);
    return r;
}

int main() {
    register_procedure("fsMount", 0, fsMount);

#ifdef _DEBUG_1_
    printRegisteredProcedures();
#endif

    launch_server();
    return 0;
}
