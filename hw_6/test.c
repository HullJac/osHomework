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

/////File1

    int f1 = bvfs_open("file1", BVFS_WTRUNC);
    printf("open file1: %d\n", f1);

    char num[10] = "0123456789";
    int w1 = bvfs_write(f1, num, 5);
    printf("write file1: %d\n", w1);

    int cl1 = bvfs_close(f1);
    printf("close file1: %d\n", cl1);
    
/////File 2

    int f2 = bvfs_open("file2", BVFS_WTRUNC);
    printf("open file2: %d\n", f2);
    
    int cl2 = bvfs_close(f2);
    printf("close file2: %d\n", cl2);

    int det = bvfs_detach();
    printf("detatch: %d\n", det);



    return 0;
}

