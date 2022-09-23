#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>


using namespace std::chrono;
using namespace std;


#define THREAD_COUNT 8

// Helper function to print arrays
void printMatrix(int const matrix[], int const size)
{
    for (auto i = 0; i < std::min(20, size); i++)
    {
        cout << matrix[i * size];

        for (auto j = 1; j < std::min(20, size); j++)
        {
            cout << ", " << matrix[i * size + j];
        }

        cout << endl;
    }

    cout << endl << "***************************************************************************************" << endl;
}


// Transposes a matrix into another pointer.
// Splits up the rows between threads.
void transpose(int const inputMatrix[], int outputMatrix[], int const size)
{
    // Worker function to transpose the matrix.
    auto worker = [=](int const threadId, int const threadCount)
    {
        for (auto i = threadId; i < size; i += threadCount)
        {
            for (auto j = 0; j < size; j++)
            {
                outputMatrix[j * size + i] = inputMatrix[i * size + j];
            }
        }
    };

    // Start a number of workers and store them in a list.
    std::vector<std::thread> threads;
    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads.emplace_back(worker, i, THREAD_COUNT);
    }

    // And wait for them to finish.
    for (auto &thread: threads)
    {
        thread.join();
    }
}


int main()
{
    // Set up the matrix column size, and total length of the storage arrays
    unsigned long constexpr size = 1024;
    unsigned long constexpr length = size * size;

    // Worker function to fill a matrix with random values using threads.
    // Splits the matrix into blocks for each thread to calculate.
    auto randomiseWorker = [&](int const block, int const blockSize, int matrix[])
    {
        // Generate a seed for the PRNG engine with the time and thread id.
        unsigned int seed = time(nullptr) * (hash<std::thread::id>()(std::this_thread::get_id()) + 1);

        // Loop through the matrix, generating a random integer between 0 and 100 for each slot.
        for (auto i = block * blockSize; i < (block + 1) * blockSize; i++)
        {
            matrix[i] = rand_r(&seed) % 100;
        }
    };

    // Worker function to calculate the matrix multiplication of matrices.
    // Each row will be calculated by a different thread, cyclically.
    auto multiplyWorker = [&](
            int const threadId, int const assignedThreads, int const matrix1[], int const matrix2Transposed[],
            int matrix3[], int const size
    )
    {
        // i represents the row and j represents the column of the output matrix that is being calculated.
        for (auto i = threadId; i < size; i += assignedThreads)
        {
            for (auto j = 0; j < size; j++)
            {
                // Sum up the multiplication of row and column of the input matrices.
                int temp = 0;
                for (auto k = 0; k < size; k++)
                {
                    temp += matrix1[i * size + k] * matrix2Transposed[j * size + k];
                }
                matrix3[i * size + j] = temp;
            }
        }
    };

    // Allocate memory for the matrices
    int *m1 = new int[length];
    int *m2 = new int[length];
    int *m3 = new int[length];

    // Randomise the input matrices, parallelising the calculations with threads.
    {
        // Work out the block sizes for the threads.
        // We will use half the threads for each matrix so split up the work based on that.
        auto blockSize = length / (THREAD_COUNT / 2);

        // Start worker threads, splitting them 50/50 between the two matrices.
        std::vector<std::thread> threads;
        for (int i = 0; i < THREAD_COUNT / 2; i++)
        {
            threads.emplace_back(randomiseWorker, i, blockSize, m1);
        }

        for (int i = 0; i < THREAD_COUNT / 2; i++)
        {
            threads.emplace_back(randomiseWorker, i, blockSize, m2);
        }

        // Wait for all the threads to finish.
        for (auto &thread: threads)
        {
            thread.join();
        }
    }

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Compute the matrix multiplication of the matrices.
    {
        // Transpose the second matrix to make it so that it is multiplying rows by rows.
        // This further helps with caching, it uses contiguous memory instead of jumping around.
        int *m2Transposed = new int[length];
        transpose(m2, m2Transposed, size);

        // Start worker threads to compute the matrix multiplication.
        std::vector<std::thread> threads;
        for (int i = 0; i < THREAD_COUNT; i++)
        {
            threads.emplace_back(multiplyWorker, i, THREAD_COUNT, m1, m2Transposed, m3, size);
        }

        // Wait for all threads to finish.
        for (auto &thread: threads)
        {
            thread.join();
        }

        delete[] m2Transposed;
    }

    // Store the time after the execution of the algorithm.
    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm.
    auto duration = duration_cast<microseconds>(stop - start);

    // Print the results.
    printMatrix(m1, size);
    printMatrix(m2, size);
    printMatrix(m3, size);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    delete[] m1;
    delete[] m2;
    delete[] m3;

    return 0;
}