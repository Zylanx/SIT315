__kernel void matrix_multiply(__global int* matrix1,
                              __global int* matrix2Transposed,
                              const int rows,
                              const int cols,
                              __global int* results)
{
    // Get the offset, and from that the row and column in the result matrix to compute for
    size_t offset = get_global_id(0);
    size_t row = offset / cols;
    size_t col = offset % cols;

    // Loop through the given column of the second matrix and the given row of the first matrix, summing the multiplications
    // into the result accumulator
    int result = 0;
    for (int i = 0; i < cols; i++) {
        result += matrix1[row * cols + i] * matrix2Transposed[col * cols + i];
    }

    // Save the result back to the offset
    results[offset] = result;
}