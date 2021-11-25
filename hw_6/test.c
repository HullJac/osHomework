#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "bvfs.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <partition name to read/write>\n", argv[0]);
        return 1;
    }

    // Get partition name from command line argument
    char *partitionName = argv[1];

    int in = bvfs_init(partitionName);
    printf("init: %d\n", in);

    int fd = bvfs_open("file", BVFS_WTRUNC);
    printf("open: %d\n", fd);

    int det = bvfs_detach();
    printf("detatch: %d\n", det);

    //bvfs_open("firstFile", BVFS_WTRUNC);


    return 0;
}

