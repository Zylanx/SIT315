#ifndef TASK1_MATRIXMULTIPLYCL_H
#define TASK1_MATRIXMULTIPLYCL_H

#include <string>
#include <CL/cl.h>

#include "types.h"


// OpenCL Matrix Multiplication class.
// It manages the initialisation of OpenCL on construction, then deals with running and returning results.
class MatrixMultiplyCl {
private:
    // All of the base OpenCL objects
    cl_device_id deviceId;
    cl_context context;
    cl_kernel kernel;
    cl_command_queue queue;
    cl_program program;

    // The memory used for matrix buffering
    cl_mem matrix1;
    cl_mem matrix2Transposed;
    cl_mem results;

    // The private methods for initialising OpenCL
    void select_device();
    void create_context();
    void create_queue();
    void create_kernel(std::string const &kernelName);
    void build_program(std::string const &filename);

public:
    // Initialises the matrix multiply with a .cl file and kernel function name
    static MatrixMultiplyCl(std::string const &filename, std::string const &kernelName);

    // Destructor manages cleaning up the OpenCL objects and memory
    ~MatrixMultiplyCl();

    // Processes the given matrices and gives an output
    void process_matrices(matrix_t matrix1[], matrix_t matrix2Transposed[], matrix_t results[], my_size_t rows, my_size_t cols);
};


#endif