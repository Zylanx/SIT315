#ifndef TASK1_MULTIPLY_CL_H
#define TASK1_MULTIPLY_CL_H


#include <string>


const std::string multiply_cl = R"(__kernel void matrix_multiply(__global int* matrix1,
                                                                __global int* matrix2Transposed,
                                                                __global unsigned int rows,
                                                                __global unsigned int cols,
                                                                __global int* results)
{
    size_t row = get_global_id(0);

    for (auto i = 0; i < cols; i++) {
        int result = 0;

        for (auto j = 0; j < rows; j++) {
            result += matrix1[row * cols + j] * matrix2Transpose[i * size + j];
        }

        results[row * cols + i] = result;
    }
})";

#endif