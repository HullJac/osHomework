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

    //bvfs_open("file", BVFS_WTRUNC);
    bvfs_init(partitionName);
    //bvfs_open("firstFile", BVFS_WTRUNC);


    return 0;
}

