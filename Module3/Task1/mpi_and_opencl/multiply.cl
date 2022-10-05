__kernel void matrix_multiply(__global int* matrix1,
                              __global int* matrix2Transposed,
                              const int rows,
                              const int cols,
                              __global int* results)
{
    size_t offset = get_global_id(0);
    size_t row = offset / cols;
    size_t col = offset % cols;

    int result = 0;
    for (int i = 0; i < cols; i++) {
        result += matrix1[row * cols + i] * matrix2Transposed[col * cols + i];
    }

    results[offset] = result;

    /*for (int i = 0; i < cols; i++) {
        int result = 0;

        for (int j = 0; j < cols; j++) {
            result += matrix1[row * cols + j] * matrix2Transposed[i * cols + j];
        }

        results[row * cols + i] = result;
    }*/
}