#include <random>
#include <algorithm>
#include <chrono>
#include <mpi.h>

#include "types.h"
#include "MatrixMultiplyCl.h"
#include "multiply_cl.h"


//#define PRINT_INPUTS_AND_OUTPUTS  // If the input and output matrices should be printed
//#define DEBUG
#define UNCOUNTED_TRANSPOSE  // If the transpose should happen before the timer or after
#define NON_ROOT_PRIORITY  // If the remaining rows should be assigned with priority to non-root nodes


// The size of the rows and columns of the matrix
constexpr my_size_t MATRIX_SIZE = 1024;


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

void multiply(int rank, int matrix1[], int matrix2[], int resultMatrix[], my_size_t size) {
#ifndef UNCOUNTED_TRANSPOSE
    if (rank == 0) {
        transpose_matrix(matrix2, size);
    }
#endif

    // broadcast matrix2
    MPI::COMM_WORLD.Bcast(matrix2, size * size, MPI::INT, 0);

    // init the displs matrix
    auto groupSize = MPI::COMM_WORLD.Get_size();
    int counts[groupSize];
    int displs[groupSize];

    // Set up the displs matrix if this is the root node
    if (rank == 0) {
        setup_scatter_arrays(size, groupSize, counts, displs);

#ifdef DEBUG
        print_var("groupSize", groupSize);
        print_vector("counts", counts, groupSize);
        print_vector("displs", displs, groupSize);
#endif
    }

    // Broadcast the displs matrix
    MPI::COMM_WORLD.Bcast(counts, groupSize, MPI::INT, 0);

    MPI::COMM_WORLD.Scatterv(matrix1, counts, displs, MPI::INT, matrix1, size * size, MPI::INT, 0);

    if (counts[rank] > 0) {
        MatrixMultiplyCl matrixMultiplyCl = MatrixMultiplyCl::from_file("multiply.cl", "matrix_multiply");

        matrixMultiplyCl.process_matrices(matrix1, matrix2, resultMatrix, counts[rank] / size, size);
    }

//    for (int i = 0; i < groupSize; i++) {
//        MPI::COMM_WORLD.Barrier();
//        if (i == rank) {
//            print_matrix("result - " + rank, resultMatrix, size);
//        }
//    }

    MPI::COMM_WORLD.Gatherv(resultMatrix, counts[rank], MPI::INT, resultMatrix, counts, displs, MPI::INT, 0);
}

int main(int argc, char *argv[])
{
    MPI::Init(argc,  argv);

    auto size = MATRIX_SIZE;

    MPI::COMM_WORLD.Bcast(&size, 1, MPI::INT, 0);

    int rank = MPI::COMM_WORLD.Get_rank();

    if (size < MPI::COMM_WORLD.Get_size()) {
        if (MPI::COMM_WORLD.Get_rank() == 0) {
            std::cerr << "Matrix smaller than number of nodes. Impossible to continue!" << std::endl <<
                      "A bug in OpenMPI 4.X means that Scatterv fails if not all nodes have work." << std::endl;
        }

        MPI::Finalize();

        return -1;
    }

    auto *matrix1 = new matrix_t[size * size]();
    auto *matrix2 = new matrix_t[size * size]();
    auto *resultMatrix = new matrix_t[size * size]();

    if (rank == 0) {
        randomise_matrix(matrix1, size);
        randomise_matrix(matrix2, size);

#ifdef PRINT_INPUTS_AND_OUTPUTS
        print_matrix("matrix1", matrix1, size);
        print_matrix("matrix2", matrix2, size);
#endif

#ifdef UNCOUNTED_TRANSPOSE
        transpose_matrix(matrix2, size);
#endif

        auto start = std::chrono::high_resolution_clock::now();

        multiply(rank, matrix1, matrix2, resultMatrix, size);

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);

#ifdef PRINT_INPUTS_AND_OUTPUTS
        print_matrix("resultMatrix", resultMatrix, size);
#endif

        std::cout << std::endl << "Total Time Taken: " << duration.count() << " microseconds" << std::endl;
    }
    else {
        auto start = std::chrono::high_resolution_clock::now();
        multiply(rank, matrix1, matrix2, resultMatrix, size);
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << std::endl << "Node Total Time Taken: " << duration.count() << " microseconds" << std::endl;
    }

    delete[] matrix1;
    delete[] matrix2;
    delete[] resultMatrix;

    MPI::Finalize();
    return 0;
}