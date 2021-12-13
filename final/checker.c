#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>

int main(int argc, char* argv[]) {
    char* fileName;

    fileName = argv[1];

    FILE* inFile = fopen(fileName, "rb+");

    uint32_t* arr = (uint32_t*) malloc(sizeof(uint32_t) * 167772165);

    fread(arr, sizeof(uint32_t), 167772165, inFile);

    for (uint32_t i = 0; i < 167772165; i++) {
        printf("%d\n", arr[i]);
    }

    return 0;
}
