#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <omp.h>


#define RUNS 500  // The number of runs to average the results over.
#define SIZE 512  // The size of the matrix.
#define THREAD_COUNT 16  // The number of threads to use.


using namespace std::chrono;
using namespace std;


#pragma region OMP Version

microseconds ompRun(unsigned long size, unsigned long length)
{
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

    delete[] m1;
    delete[] m2;
    delete[] m3;
    delete[] m2Transposed;

    return duration;
}

#pragma endregion


#pragma region std::thread Version

// Transposes a matrix into another pointer.
// Splits up the rows between threads.
void transposeStdThread(int const inputMatrix[], int outputMatrix[], int const size)
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


microseconds stdThreadRun(unsigned long size, unsigned long length)
{
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
        transposeStdThread(m2, m2Transposed, size);

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

    delete[] m1;
    delete[] m2;
    delete[] m3;

    return duration;
}

#pragma endregion


#pragma region Sequential Version

// Transposes a matrix into another pointer.
void transposeSequential(int const inputMatrix[], int outputMatrix[], int const size)
{
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            outputMatrix[j * size + i] = inputMatrix[i * size + j];
        }
    }
}


microseconds sequentialRun(unsigned long size, unsigned long length)
{
    // Seed the PRNG engine.
    srand(time(nullptr));

    // Allocate memory for the matrices
    // The matrices will be stored as 1D arrays, instead of 2D arrays, to help with caching.
    int *m1 = (int *)malloc(sizeof(int *) * length);
    int *m2 = (int *)malloc(sizeof(int *) * length);
    int *m3 = (int *)malloc(sizeof(int *) * length);

    // Loop through the input matrices, generating random integer between 0 and 100 for each slot.
    for (int i = 0; i < length; i++)
    {
        m1[i] = rand() % 100;
        m2[i] = rand() % 100;
    }

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Transpose the second matrix to make it so that it is multiplying rows by rows.
    // This further helps with caching, it uses contiguous memory instead of jumping around.
    int *m2Transposed = new int[length];
    transposeSequential(m2, m2Transposed, size);

    // Compute the vector addition for each element of the input matrices.
    // i represents the row and j represents the column of the output matrix that is being calculated.
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            // Sum up the multiplication of row and column of the input matrices.
            int temp = 0;
            for (int k = 0; k < size; k++)
            {
                temp += m1[i * size + k] * m2Transposed[j * size + k];
            }
            m3[i * size + j] = temp;
        }
    }

    // Store the end time of the algorithm.
    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

    return duration;
}

#pragma endregion


// Return the average run time of a list of runs.
unsigned long average(vector<microseconds> runs)
{
    return (reduce(runs.begin(), runs.end()) / runs.size()).count();
}


int main()
{
    // Set up the matrix column size, and total length of the storage arrays
    unsigned long constexpr size = SIZE;
    unsigned long constexpr length = size * size;

    // Initialise a spaces string to blank out a console line.
    int line_width = getenv("COLUMNS") ? atoi(getenv("COLUMNS")) : 80;
    string spaces(line_width - 1, ' ');

    // Set fields to be left justified
    cout << left;

    // Run each algorithm a number of times, storing the run times and displaying the running average.
    // Later they will be used to show a final comparison.
    cout << "Sequential Runs" << endl;
    vector<microseconds> sequentialRuns;
    for (auto i = 0; i < RUNS / 10; i++)
    {
        sequentialRuns.push_back(sequentialRun(size, length));

        cout << '\r' << spaces << '\r' << "Runs: " << setw(5) << sequentialRuns.size() << "Average: "
             << average(sequentialRuns) << flush;
    }
    cout << endl << "------------------------------" << endl;

    cout << "std::thread Runs" << endl;
    vector<microseconds> stdThreadRuns;
    for (auto i = 0; i < RUNS; i++)
    {
        stdThreadRuns.push_back(stdThreadRun(size, length));

        cout << '\r' << spaces << '\r' << "Runs: " << setw(5) << stdThreadRuns.size() << "Average: "
             << average(stdThreadRuns) << flush;
    }
    cout << endl << "------------------------------" << endl;

    cout << "OMP Runs" << endl;
    vector<microseconds> ompRuns;
    for (auto i = 0; i < RUNS; i++)
    {
        ompRuns.push_back(ompRun(size, length));

        cout << '\r' << spaces << '\r' << "Runs: " << setw(5) << ompRuns.size() << "Average: "
             << average(ompRuns) << flush;
    }
    cout << endl << "------------------------------" << endl << endl;

    // Print out all the averages for comparison.
    cout << "Sequential Average: " << average(sequentialRuns) << endl;
    cout << "std::thread Average: " << average(stdThreadRuns) << endl;
    cout << "OMP Average: " << average(ompRuns) << endl;

    return 0;
}