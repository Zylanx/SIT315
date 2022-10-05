#include "MatrixMultiplyCl.h"


#include <iostream>
#include <fstream>


MatrixMultiplyCl::~MatrixMultiplyCl() {
    clReleaseMemObject(this->matrix1);
    clReleaseMemObject(this->matrix2Transposed);
    clReleaseMemObject(this->results);

    clReleaseKernel(this->kernel);
    clReleaseCommandQueue(this->queue);
    clReleaseProgram(this->program);
    clReleaseContext(this->context);
}

void MatrixMultiplyCl::select_device() {
    cl_platform_id platform;
    int err;

    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err < 0) {
        std::cerr << "Couldn't identify a platform" << std::endl;
        exit(err);
    }

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &this->deviceId, nullptr);
    if (err == CL_DEVICE_NOT_FOUND) {
        std::cerr << "GPU not found, searching for CPU" << std::endl;

        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &this->deviceId, nullptr);
    }

    if (err < 0) {
        std::cerr << "Couldn't find any valid devices" << std::endl;
        exit(err);
    }
}

void MatrixMultiplyCl::create_context() {
    int err;

    this->context = clCreateContext(nullptr, 1, &this->deviceId, nullptr, nullptr, &err);
    if (err < 0) {
        std::cerr << "Couldn't create a context" << std::endl;
        exit(err);
    }
}

void MatrixMultiplyCl::create_queue() {
    int err;

    this->queue = clCreateCommandQueueWithProperties(this->context, this->deviceId, nullptr, &err);
    if (err < 0) {
        std::cerr << "Couldn't create a command queue" << std::endl;
        exit(err);
    }
}

void MatrixMultiplyCl::create_kernel(std::string const &kernelName) {
    int err;

    this->kernel = clCreateKernel(this->program, kernelName.c_str(), &err);
    if (err < 0) {
        std::cerr << "Couldn't create a kernel" << std::endl;
        exit(err);
    }
}

void MatrixMultiplyCl::build_program(std::string const &filename) {
    std::ifstream programFile;
    int err;

    try {
        programFile.open(filename);
    }
    catch(std::ios_base::failure& exc) {
        std::cerr << exc.what() << std::endl;
        throw;
    }

    // Get the size
    programFile.seekg(0, std::ios_base::end);
    size_t programSize = programFile.tellg();

    // Go back to the start
    programFile.seekg(0);

    // Create the buffer source and null-terminate it
    char *sourceBuffer = new char[programSize + 1];
    sourceBuffer[programSize] = '\0';

    // Read out the source into the buffer
    programFile.read(sourceBuffer, (std::streamsize)programSize);
    programFile.close();

    // load the program for the context
    this->program = clCreateProgramWithSource(this->context, 1, (const char **)&sourceBuffer, &programSize, &err);
    if (err < 0) {
        std::cout << "Program failed to compile" << std::endl;
        exit(err);
    }
    delete[] sourceBuffer;

    err = clBuildProgram(this->program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err < 0) {
        size_t logSize;

        clGetProgramBuildInfo(this->program, this->deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        char* programLog = new char[logSize + 1];
        clGetProgramBuildInfo(this->program, this->deviceId, CL_PROGRAM_BUILD_LOG, logSize + 1, programLog, nullptr);
        std::cerr << programLog << std::endl;

        delete[] programLog;

        exit(err);
    }
}

void MatrixMultiplyCl::build_program_from_string(std::string const &source) {
    int err;

    // Get the size
    size_t programSize = source.length();

    // load the program for the context
    this->program = clCreateProgramWithSource(this->context, 1, (const char **)source.c_str(), &programSize, &err);
    if (err < 0) {
        std::cout << "Program failed to compile" << std::endl;
        exit(err);
    }

    err = clBuildProgram(this->program, 0, nullptr, nullptr, nullptr, nullptr);
    if (err < 0) {
        size_t logSize;

        clGetProgramBuildInfo(this->program, this->deviceId, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        char* programLog = new char[logSize + 1];
        clGetProgramBuildInfo(this->program, this->deviceId, CL_PROGRAM_BUILD_LOG, logSize + 1, programLog, nullptr);
        std::cerr << programLog << std::endl;

        delete[] programLog;

        exit(err);
    }
}

MatrixMultiplyCl MatrixMultiplyCl::from_file(std::string const &filename, std::string const &kernelName) {
    MatrixMultiplyCl newObject{};

    newObject.select_device();

    newObject.create_context();

    newObject.build_program(filename);

    newObject.create_queue();
    newObject.create_kernel(kernelName);

    return newObject;
}

MatrixMultiplyCl MatrixMultiplyCl::from_string(std::string const &source, std::string const &kernelName) {
    MatrixMultiplyCl newObject{};

    newObject.select_device();

    newObject.create_context();

    newObject.build_program_from_string(source);

    newObject.create_queue();
    newObject.create_kernel(kernelName);

    return newObject;
}

void MatrixMultiplyCl::process_matrices(matrix_t matrix1[], matrix_t matrix2Transposed[], matrix_t results[], my_size_t rows, my_size_t cols) {
    this->matrix1 = clCreateBuffer(context, CL_MEM_READ_ONLY, rows * cols * sizeof(int), nullptr, nullptr);
    this->matrix2Transposed = clCreateBuffer(context, CL_MEM_READ_ONLY, cols * cols * sizeof(int), nullptr, nullptr);
    this->results = clCreateBuffer(context, CL_MEM_WRITE_ONLY, rows * cols * sizeof(int), nullptr, nullptr); // TODO: Size here might not be right.

    clEnqueueWriteBuffer(this->queue, this->matrix1, CL_TRUE, 0, rows * cols * sizeof(int), matrix1, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(this->queue, this->matrix2Transposed, CL_TRUE, 0, cols * cols * sizeof(int), matrix2Transposed, 0, nullptr, nullptr);
    clEnqueueWriteBuffer(this->queue, this->results, CL_TRUE, 0, rows * cols * sizeof(int), results, 0, nullptr, nullptr);

    clSetKernelArg(this->kernel, 0, sizeof(cl_mem), (void *)&this->matrix1);
    clSetKernelArg(this->kernel, 1, sizeof(cl_mem), (void *)&this->matrix2Transposed);
    clSetKernelArg(this->kernel, 2, sizeof(int), (void *)&rows);
    clSetKernelArg(this->kernel, 3, sizeof(int), (void *)&cols);
    clSetKernelArg(this->kernel, 4, sizeof(cl_mem), (void *)&this->results);

    cl_event event = nullptr;
    size_t global[1] = {(size_t)rows * cols};

    clEnqueueNDRangeKernel(this->queue, this->kernel, 1, nullptr, global, nullptr, 0, nullptr, &event);
    clWaitForEvents(1, &event);

    clEnqueueReadBuffer(this->queue, this->results, CL_TRUE, 0, rows * cols * sizeof(int), results, 0, nullptr, nullptr);
}