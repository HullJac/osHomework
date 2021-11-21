#include <pthread.h>
// Ending size should be mat1_m * mat2_n
// Struct to pass to all the threads
struct tMatrix {
    int* mat_1;
    int* mat_2;
    int* mat_3;
    int mat1_r;
    int mat2_r;
    int mat1_c;
    int mat2_c;
    int flag;
};

// Do matrix multiplication
void* matrixPiece(void* arg) {
    struct tMatrix* data = (struct tMatrix*)arg;
    // Reassigning variable names
    int* Mat1 = data->mat_1;
    int* Mat2 = data->mat_2;
    int* Mat3 = data->mat_3;
    int mat1_row = data->mat1_r;
    int mat2_row = data->mat2_r;
    int mat1_col = data->mat1_c;
    int mat2_col = data->mat2_c;
    int Flag = data->flag;
    // Create starting points and incrementers
    int iInc = 2;
    int jInc = 2;
    int row = 0;
    int col = 0;
    // Odd row to even columns
    if (Flag == 1){
       row = 1;
    }
    // Even rows to odd columns
    else if (Flag == 2) {
        col = 1;
    }
    // Odd rows onto odd columns
    else if (Flag == 3) {
        row = 1;
        col = 1;
    }
    // Else do even rows onto even columns
    // Same matrix multiplication as from ex_1.h
    for (int i = row; i < mat1_row; i+=iInc) {
        for (int j = col; j < mat2_col; j+=jInc) {
            int cell = 0;
            for (int x = 0; x < mat1_col; x++) {
                cell += Mat1[(i * mat1_col) + x] * Mat2[(x * mat2_col) + j];
            }
            Mat3[(i * mat2_col) + j] = cell;
        }
    }
    return NULL;
}

// Runs the threading
void m_mult_threaded(int *mat1, int mat1_m, int mat1_n,
        int *mat2, int mat2_m, int mat2_n,
        int *mat3)
{
    // Divide work up here and make structs
    struct tMatrix s_list[4];
    struct tMatrix s0 = {mat1, mat2, mat3, mat1_m, mat2_m, mat1_n, mat2_n, 1};
    struct tMatrix s1 = {mat1, mat2, mat3, mat1_m, mat2_m, mat1_n, mat2_n, 2};
    struct tMatrix s2 = {mat1, mat2, mat3, mat1_m, mat2_m, mat1_n, mat2_n, 3};
    struct tMatrix s3 = {mat1, mat2, mat3, mat1_m, mat2_m, mat1_n, mat2_n, 4};
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
