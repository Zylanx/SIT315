#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <chrono>
#include <iostream>

#define PRINT 1

int SZ = 1000000;
int *v1;
int *v2;

// OpenCL memory object used to share v with the device and to work on the result
cl_mem bufV1;
cl_mem bufV2;

// This is the ID of the device selected for use
cl_device_id device_id;
// The created context for the device selected
cl_context context;
// The compiled program to be executed in parallel
cl_program program;
// The kernel function of that program to be executed
cl_kernel kernel;
// The command queue for the device
cl_command_queue queue;
cl_event event = NULL;

int err;

// Finds a valid device to use and returns its ID
cl_device_id create_device();
// Sets up a context for a given kernel, detecting the platform and device to use automatically.
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
// Builds the program from the file for the context and device.
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
// Creates a memory buffer for the kernel and pushes data to it
void setup_kernel_memory();
// Copies function arguments to the kernel function
void copy_kernel_args();
// Free's all the dynamic memory leftover
void free_memory();

void init(int *&A, int size);
void print(int *A, int size);

int main(int argc, char **argv)
{
    if (argc > 1)
        SZ = atoi(argv[1]);

    init(v1, SZ);
    init(v2, SZ);

    // The size of the vector, as a size_t
    size_t global[1] = {(size_t)SZ};

    // initial vector
    print(v1, SZ);
    print(v2, SZ);

    setup_openCL_device_context_queue_kernel((char *)"./vector_ops.cl", (char *)"add_vectors");

    setup_kernel_memory();
    copy_kernel_args();

    auto start = std::chrono::high_resolution_clock::now();

    // Add a command to the queue to execute the given kernel on its device then wait for it to finish.
    //
    // The rest of the arguments define the arrays that are used to specify information about the
    // work items and groups that will be executed
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    // Adds a command to read the result array from the buffer back into v
    //
    // blocking_read is set to true to make sure it is synchronous.
    // The other arguments define where in the buffer to read and where to save
    clEnqueueReadBuffer(queue, bufV1, CL_TRUE, 0, SZ * sizeof(int), &v1[0], 0, NULL, NULL);

    auto duration = std::chrono::high_resolution_clock::now() - start;

    // result vector
    print(v1, SZ);

    std::cout << std::endl << "Elapsed time: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << " microseconds" << std::endl;

    // frees memory for device, kernel, queue, etc.
    // you will need to modify this to free your own buffers
    free_memory();
}

void init(int *&A, int size)
{
    A = (int *)malloc(sizeof(int) * size);

    for (long i = 0; i < size; i++)
    {
        A[i] = rand() % 100; // any number less than 100
    }
}

void print(int *A, int size)
{
    if (PRINT == 0)
    {
        return;
    }

    if (PRINT == 1 && size > 15)
    {
        for (long i = 0; i < 5; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
        printf(" ..... ");
        for (long i = size - 5; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    else
    {
        for (long i = 0; i < size; i++)
        {                        //rows
            printf("%d ", A[i]); // print the cell value
        }
    }
    printf("\n----------------------------\n");
}

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufV1);
    clReleaseMemObject(bufV2);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v1);
    free(v2);
}


void copy_kernel_args()
{
    // Set the arguments passed to the kernel
    // The first argument is the kernel function, the second is the index of the argument
    // The final argument is the value to be passed
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV1);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&bufV2);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory()
{
    // Create buffers to hold the input vectors
    // It is created as a read write buffer, meaning it can be used as a general storage.
    bufV1 = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);
    bufV2 = clCreateBuffer(context, CL_MEM_READ_ONLY, SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufV1, CL_TRUE, 0, SZ * sizeof(int), &v1[0], 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, bufV2, CL_TRUE, 0, SZ * sizeof(int), &v2[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    // Create the context for the given device
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        printf("Error code = %i\n", err);
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    // Create a command queue for a specific device
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };


    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    /* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    // Loads a program and compiles it for a given context
    program = clCreateProgramWithSource(ctx, 1,
                                        (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      printf("Error code = %i\n", err);
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      printf("GPU not found\n");
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}
