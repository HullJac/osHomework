#include <pthread.h>
#include <stdio.h>
// Ending size should be mat1_m * mat2_n

// Global variables to help with threaded function
int mat1_col = 0;
int mat2_col = 0;

// Struct to pass to all the threads
struct tMatrix {
    int* mat_1;
    int* mat_2;
    int* mat_3;
    int i_start;
    int j_start;
    int i_end;
    int j_end;
};

// Do matrix multiplication
void* matrixPiece(void* arg) {
    struct tMatrix* data = (struct tMatrix*)arg;

    int* Mat1 = data->mat_1;
    int* Mat2 = data->mat_2;
    int* Mat3 = data->mat_3;
    int iStart = data->i_start;
    int jStart = data->j_start;
    int iEnd = data->i_end;
    int jEnd = data->j_end;

    for (iStart; iStart < iEnd; iStart++) {
        for (jStart; jStart < jEnd; jStart++) {
            int cell = 0;
            for (int x = 0; x < mat1_col; x++) {
                cell += Mat1[(iStart * mat1_col) + x] * Mat2[(x * mat2_col) + jStart];
            }
            Mat3[(iStart * mat2_col) + jStart] = cell;
        }
    }
    return NULL;
}

// Runs the threading
void m_mult_threaded(int *mat1, int mat1_m, int mat1_n,
                     int *mat2, int mat2_m, int mat2_n,
                     int *mat3)
{
    // Assign global variables that we will always use
    mat1_col = mat1_n;
    mat2_col = mat2_n;

    int endi0, endj0, starti1, startj1, endi1, endj1;
    int starti2, startj2, endi2, endj2, starti3, startj3;

    // Number of things to divide among the threads
    int squares = mat1_m * mat2_n;
    int extra = squares % 4;
    if (extra == 0) {
        // This is the gap between start and end for each thread
        int gap = squares / 4;
        
        endi0 = (gap-1) / mat2_n;
        endj0 = (gap-1) % mat2_n;
        printf("%d",endi0);
        printf("%d",endj0);

        starti1 = gap / mat2_n;
        startj1 = gap % mat2_n;
        printf("%d",starti1);
        printf("%d",startj1);

        endi1 = ((gap*2)-1) / mat2_n;
        endj1 = ((gap*2)-1) % mat2_n;
        printf("%d",endi1);
        printf("%d",endj1);
        
        starti2 = (gap*2) / mat2_n;
        startj2 = (gap*2) % mat2_n;
        printf("%d",starti2);
        printf("%d",startj2);
        
        endi2 = ((gap*3)-1) / mat2_n;
        endj2 = ((gap*3)-1) % mat2_n;
        printf("%d",endi2);
        printf("%d",endj2);
        
        starti3 = (gap*3) / mat2_n;
        startj3 = (gap*3) % mat2_n;
        printf("%d",starti3);
        printf("%d",startj3);
    }
    else if (extra == 1) {
        int gap = squares / 4;
        endi0 = gap / mat2_n; 
        endj0 = gap % mat2_n;
        
        starti1 = (gap+1) / mat2_n;
        startj1 = (gap+1) % mat2_n; 

        endi1 = (gap*2) / mat2_n;
        endj1 = (gap*2) % mat2_n;
        
        starti2 = ((gap*2)+1) / mat2_n;
        startj2 = ((gap*2)+1) % mat2_n; 
        
        endi2 = (gap*3) / mat2_n;
        endj2 = (gap*3) % mat2_n;
        
        starti3 = ((gap*3)+1) / mat2_n;
        startj3 = ((gap*3)+1) % mat2_n; 
    }
    else if (extra == 2) {
        int gap = squares / 4;
        endi0 = gap / mat2_n; 
        endj0 = gap % mat2_n;
        
        starti1 = (gap+1) / mat2_n;
        startj1 = (gap+1) % mat2_n; 

        endi1 = (gap*2) / mat2_n;
        endj1 = (gap*2) % mat2_n;
        
        starti2 = ((gap*2)+1) / mat2_n;
        startj2 = ((gap*2)+1) % mat2_n; 
        
        endi2 = (gap*3) / mat2_n;
        endj2 = (gap*3) % mat2_n;
        
        starti3 = ((gap*3)+1) / mat2_n;
        startj3 = ((gap*3)+1) % mat2_n; 
    }
    else if (extra == 3) {
        int gap = squares / 4;
        endi0 = gap / mat2_n; 
        endj0 = gap % mat2_n;
        
        starti1 = (gap+1) / mat2_n;
        startj1 = (gap+1) % mat2_n; 

        endi1 = (gap*2) / mat2_n;
        endj1 = (gap*2) % mat2_n;
        
        starti2 = ((gap*2)+1) / mat2_n;
        startj2 = ((gap*2)+1) % mat2_n; 
        
        endi2 = (gap*3) / mat2_n;
        endj2 = (gap*3) % mat2_n;
        
        starti3 = ((gap*3)+1) / mat2_n;
        startj3 = ((gap*3)+1) % mat2_n; 
    }
    // Divide work up here and make structs
    struct tMatrix s_list[4];
    struct tMatrix s0 = {mat1, mat2, mat3, 0, 0, endi0, endj0};
    struct tMatrix s1 = {mat1, mat2, mat3, starti1, startj1, endi1, endj1};
    struct tMatrix s2 = {mat1, mat2, mat3, starti2, startj2, endi2, endj2};
    struct tMatrix s3 = {mat1, mat2, mat3, starti3, startj3, mat1_m, mat2_n};
    //struct tMatrix s0 = {mat1, mat2, mat3, 0, 0, 1, 1};
    //struct tMatrix s1 = {mat1, mat2, mat3, 0, 1, 1, 2};
    //struct tMatrix s2 = {mat1, mat2, mat3, 1, 0, 2, 1};
    //struct tMatrix s3 = {mat1, mat2, mat3, 1, 1 ,mat1_m ,mat2_n};
    s_list[0] = s0;
    s_list[1] = s1;
    s_list[2] = s2;
    s_list[3] = s3;
    
    // Create threads
    pthread_t tID[4];
    for (int i = 0; i < 4; i++) {
        // Launch threads
        pthread_create(&(tID[i]), NULL, matrixPiece, &(s_list[i]));
    }

    // Joining the threads
    for (int i = 0; i < 4; i++) {
        pthread_join(tID[i], NULL);
    }

}
