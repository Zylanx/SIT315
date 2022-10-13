#include <random>
#include <algorithm>
#include <chrono>
#include <mpi.h>

#include "MatrixMultiplyCl.h"

//#define PRINT_INPUTS_AND_OUTPUTS  // If the input and output matrices should be printed
//#define DEBUG
#define UNCOUNTED_TRANSPOSE  // If the transpose should happen before the timer or after
#define NON_ROOT_PRIORITY  // If the remaining rows should be assigned with priority to non-root nodes


// Type aliases for our usage
using matrix_t = int;
using my_size_t = int;


// The size of the rows and columns of the matrix
constexpr my_size_t MATRIX_SIZE = 4096;


// Print out a matrix, along with its name, to the given output.
void print_matrix(std::string const& name, const matrix_t matrix[], my_size_t const& size, std::ostream& stream = std::cout) {
    stream << "============= " << name << " =============" << std::endl;

    // loop through the matrix, printing separators and newlines.
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            stream << matrix[i * size + j];

            // Skip the separator for the last element in the row.
            if (j < (size - 1)) {
                stream << ", ";
            }
        }

        stream << std::endl;
    }
}

// Prints a vector, along with its name, to the given output.
void print_vector(std::string const& name, matrix_t const matrix[], my_size_t const& size, std::ostream& stream = std::cout) {
    stream << "============= " << name << " =============" << std::endl;

    // loop through the vector, printing separators.
    for (int i = 0; i < size; i++) {
        stream << matrix[i];

        // Skip the separator for the last element.
        if (i < (size - 1)) {
            stream << ", ";
        }
    }

    stream << std::endl;
}

// Randomise the input matrix
void randomise_matrix(matrix_t matrix[], my_size_t const& size)
{
    // Seed a PRNG with a CRNG provided by the system.
    std::random_device randomDevice;
    std::mt19937 generator(randomDevice());

    std::uniform_int_distribution<> distribution(0, 100);

    // Loop through the matrix, randomly generating elements.
    for (auto i = 0; i < size * size; i++)
    {
        matrix[i] = distribution(generator);
    }
}

// Templated function to print out a variable with its name.
template <typename T>
void print_var(std::string const& name, T value) {
    std::cout << std::endl << name << ": " << value << std::endl;
}

// Transposes the input matrix
// Helps with caching by keeping the access sequential when accessing
// what would originally be the columns.
void transpose_matrix(matrix_t matrix[], my_size_t const& size) {
    // Create a temporary matrix for the intermediate result.
    auto *matrixTemp = new matrix_t[size * size]();

    // Loop through the matrix, transposing it into the temp matrix
    for (auto i = 0; i < size; i++) {
        for (auto j = 0; j < size; j++) {
            matrixTemp[j * size + i] = matrix[i * size + j];
        }
    }

    // Copy the transposed matrix back into the original matrix
    for (auto i = 0; i < size * size; i++) {
        matrix[i] = matrixTemp[i];
    }

    delete[] matrixTemp;
}

// Set up the control arrays for Scatterv
// Tries to evenly spread the rows around as evenly as possible.
void setup_scatter_arrays(my_size_t const& size, int const groupSize, int counts[], int displs[]) {
    // Work out the number of rows per process, rounded down, as well as the remaining rows.
    auto rowsPerProcess = size / groupSize;
    auto remainingRows = size % groupSize;

#ifdef DEBUG
    // Print out some debug info regarding how the matrix is being split up
    print_var("rowsPerProcess", rowsPerProcess);
    print_var("remainingRows", remainingRows);
#endif

    // Set up the sendcounts array, it specifies how many elements each node receives.
    {
        // First assign out the evenly spread rows.
        for (auto i = 0; i < groupSize; i++) {
            counts[i] = rowsPerProcess * size;
        }

        // Assign the rest of the rows by repeatedly looping through the nodes, giving
        // each node a row each time.
        while (remainingRows > 0) {
            for (int i = 0; i < groupSize && remainingRows > 0; i++) {
#ifndef NON_ROOT_PRIORITY
                counts[i] += size;
#else
                counts[groupSize - i - 1] += size;
#endif
                remainingRows--;
            }
        }
    }

    // Set up the displacement array.
    int counter = 0;
    for (auto i = 0; i < groupSize; i++) {
        // If the count is zero, the displacement should be the same as the previous, or 0 for the first.
        if (i > 0 && counts[i] == 0) {
            displs[i] = displs[i - 1];
        } else {
            displs[i] = counter;
            counter += counts[i];
        }
    }
}

// Multiply a matrix with a vector and save the result into another vector.
void matrix_vector_multiply(const matrix_t rowVector[], const matrix_t matrixTranspose[], matrix_t resultVector[], my_size_t size) {
#pragma omp parallel for
    // Loop through each position of the row vector
    for (auto i = 0; i < size; i++) {
        matrix_t result = 0;

        // Then loop through each position of the row vector and the corresponding matrix column (or row for the transposed matrix
        // adding the multiplication of those two elements to the accumulator
        for (auto j = 0; j < size; j++) {
            result += rowVector[j] * matrixTranspose[i * size + j];
        }

        // Save the result back
        resultVector[i] = result;
    }
}

// Multiplies two vectors using MPI and OpenMP
// The second matrix will be transposed and broadcast, then the rows of the first matrix are spread between all the
// processes in the MPI group.
void multiply(int rank, int matrix1[], int matrix2[], int resultMatrix[], my_size_t size) {
#ifndef UNCOUNTED_TRANSPOSE
    // Transpose matrix 2 in the root process.
    if (rank == 0) {
        // Transpose the matrix to make the multiplication easier, ideally it would keep the matrix in the cache more readily.
        transpose_matrix(matrix2, size);
    }
#endif

    // Broadcast matrix2
    MPI::COMM_WORLD.Bcast(matrix2, size * size, MPI::INT, 0);

    // Init the counts and displs arrays
    auto groupSize = MPI::COMM_WORLD.Get_size();
    int counts[groupSize];
    int displs[groupSize];

    // Set up the counts and displs matrix if this is the root node
    if (rank == 0) {
        setup_scatter_arrays(size, groupSize, counts, displs);

#ifdef DEBUG
        print_var("groupSize", groupSize);
        print_vector("counts", counts, groupSize);
        print_vector("displs", displs, groupSize);
#endif
    }

    // Broadcast the counts matrix
    MPI::COMM_WORLD.Bcast(counts, groupSize, MPI::INT, 0);

    // Scatter matrix one across all the processes
    MPI::COMM_WORLD.Scatterv(matrix1, counts, displs, MPI::INT, matrix1, size * size, MPI::INT, 0);

    // Process the matrix multiplications through OpenCL using our matrix multiply class
    MatrixMultiplyCl matrixMultiplyCl("multiply.cl", "matrix_multiply");
    matrixMultiplyCl.process_matrices(matrix1, matrix2, resultMatrix, counts[rank] / size, size);

    // Collect the results back
    MPI::COMM_WORLD.Gatherv(resultMatrix, counts[rank], MPI::INT, resultMatrix, counts, displs, MPI::INT, 0);
}

int main(int argc, char *argv[])
{
    MPI::Init(argc, argv);

    auto size = MATRIX_SIZE;

    // Broadcast the matrix size
    MPI::COMM_WORLD.Bcast(&size, 1, MPI::INT, 0);

    int rank = MPI::COMM_WORLD.Get_rank();

    // MPI Bug: Scatterv hangs when one process gets 0 items
    if (size < MPI::COMM_WORLD.Get_size()) {
        if (MPI::COMM_WORLD.Get_rank() == 0) {
            std::cerr << "Matrix smaller than number of nodes. Impossible to continue!" << std::endl <<
                      "A bug in OpenMPI 4.X means that Scatterv fails if not all nodes have work." << std::endl;
        }

        MPI::Finalize();

        return -1;
    }

    // Initialise the matrices
    auto *matrix1 = new matrix_t[size * size]();
    auto *matrix2 = new matrix_t[size * size]();
    auto *resultMatrix = new matrix_t[size * size]();

    // If the process is root, then randomise the matrix and then start the multiplication, or just start the multiplication
    if (rank == 0) {
        // Randomise the input matrices
        randomise_matrix(matrix1, size);
        randomise_matrix(matrix2, size);

#ifdef PRINT_INPUTS_AND_OUTPUTS
        // If we are outputting, then print the matrices
        print_matrix("matrix1", matrix1, size);
        print_matrix("matrix2", matrix2, size);
#endif

#ifdef UNCOUNTED_TRANSPOSE
        // Transpose the matrix to make the multiplication easier, ideally it would keep the matrix in the cache more readily.
        transpose_matrix(matrix2, size);
#endif

        auto start = std::chrono::high_resolution_clock::now();

        // Start the multiplication
        multiply(rank, matrix1, matrix2, resultMatrix, size);

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);

#ifdef PRINT_INPUTS_AND_OUTPUTS
        // Print the output/result matrix
        print_matrix("resultMatrix", resultMatrix, size);
#endif

        std::cout << std::endl << "Total Time Taken: " << duration.count() << " microseconds" << std::endl;
    }
    else {
        auto start = std::chrono::high_resolution_clock::now();

        // Start the multiplication
        multiply(rank, matrix1, matrix2, resultMatrix, size);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);

        std::cout << std::endl << "Node Total Time Taken: " << duration.count() << " microseconds" << std::endl;
    }

    // Free up the allocated memory
    delete[] matrix1;
    delete[] matrix2;
    delete[] resultMatrix;

    // Finalise and return
    MPI::Finalize();
    return 0;
}
