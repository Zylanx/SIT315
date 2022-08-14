#include <iostream>
#include <cstdlib>
#include <chrono>
#include <vector>
#include <omp.h>


#define SIZE 512  // The size of the matrix.
#define THREAD_COUNT 16  // The number of threads to use.


using namespace std::chrono;
using namespace std;


// Helper function to print arrays
void printMatrix(int const matrix[], int const size)
{
    for (auto i = 0; i < min(20, size); i++)
    {
        cout << matrix[i * size];

        for (auto j = 1; j < min(20, size); j++)
        {
            cout << ", " << matrix[i * size + j];
        }

        cout << endl;
    }

    cout << "***************************************************************************************" << endl;
}


int main()
{
    // Set up the matrix column size, and total length of the storage arrays
    unsigned long constexpr size = SIZE;
    unsigned long constexpr length = size * size;

    // Set the number of threads OMP can use.
    omp_set_num_threads(THREAD_COUNT);

    // Allocate memory for the matrices
    int *m1 = new int[length];
    int *m2 = new int[length];
    int *m3 = new int[length];

    // Set up an array to store the transposed version of m2.
    int *m2Transposed = new int[length];

#pragma omp parallel default(none) firstprivate(length) shared(m1, m2)
    {
        // Generate a private seed for each thread based on the time and the thread number.
        unsigned int seed = (unsigned int)omp_get_wtime() * omp_get_thread_num() + 1;

        // Loop through the matrices, generating a random integer between 0 and 100 for each slot.
#pragma omp for
        for (auto i = 0; i < length; i++)
        {
            m1[i] = rand_r(&seed) % 100;
        }
#pragma omp for
        for (auto i = 0; i < length; i++)
        {
            m2[i] = rand_r(&seed) % 100;
        }
    }

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Fork the program into multiple threads, for the main parts of the algorithm.
#pragma omp parallel default(none) firstprivate(size, length) shared(m1, m2, m3, m2Transposed)
    {
        // Transpose the second matrix to speed up the algorithm
        // Helps with caching by keeping the access sequential when accessing
        // what would originally be the columns.
#pragma omp for
        for (auto i = 0; i < size; i++)
        {
            for (auto j = 0; j < size; j++)
            {
                m2Transposed[j * size + i] = m2[i * size + j];
            }
        }

        // Compute the matrix multiplication for every element.
        // i represents the row and j represents the column of the output matrix that is being calculated.
#pragma omp for
        for (auto i = 0; i < size; i++)
        {
            for (auto j = 0; j < size; j++)
            {
                // Sum up the multiplication of row and column of the input matrices.
                int temp = 0;
                for (auto k = 0; k < size; k++)
                {
                    temp += m1[i * size + k] * m2Transposed[j * size + k];
                }
                m3[i * size + j] = temp;
            }
        }
    }

    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

    printMatrix(m1, size);
    printMatrix(m2, size);
    printMatrix(m3, size);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    delete[] m1;
    delete[] m2;
    delete[] m3;
    delete[] m2Transposed;

    return 0;
}