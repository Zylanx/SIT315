#ifndef TASK1_MATRIXMULTIPLYCL_H
#define TASK1_MATRIXMULTIPLYCL_H

#include <string>
#include <CL/cl.h>

#include "types.h"


class MatrixMultiplyCl {
private:
    cl_device_id deviceId;
    cl_context context;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_program program;

    cl_mem matrix1;
    cl_mem matrix2Transposed;
    cl_mem results;

    void select_device();
    void create_context();
    void create_queue();
    void create_kernel(std::string const &kernelName);
    void build_program(std::string const &filename);
    void build_program_from_string(std::string const &source);

public:
    ~MatrixMultiplyCl();

    static MatrixMultiplyCl from_file(std::string const &filename, std::string const &kernelName);
    static MatrixMultiplyCl from_string(std::string const &source, std::string const &kernelName);

    void process_matrices(matrix_t matrix1[], matrix_t matrix2Transposed[], matrix_t results[], my_size_t rows, my_size_t cols);
};


#endif