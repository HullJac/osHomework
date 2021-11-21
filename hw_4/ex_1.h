// Ending size should be the mat1_m * mat2_n
// I chose a triple for loop approach to this problem
void m_mult(int *mat1, int mat1_m, int mat1_n,
            int *mat2, int mat2_m, int mat2_n,
            int *mat3)
{
    for (int i = 0; i < mat1_m; i++) {
        for (int j = 0; j < mat2_n; j++) {
            // cell is the value that will go in each cell
            int cell = 0;
            for (int x = 0; x < mat1_n; x++) {
                cell += mat1[(i * mat1_n) + x] * mat2[(x * mat2_n) + j];
            }
            mat3[(i * mat2_n) + j] = cell;
        }
    }
}


