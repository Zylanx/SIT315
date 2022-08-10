#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>
#include <thread>
#include <omp.h>


#define RUNS 500
#define SIZE 512
#define THREADS omp_get_max_threads()


using namespace std::chrono;
using namespace std;

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

    cout << "***************************************************************************************" << endl;
}


// TODO: Redo the comments from vector to matrix


void TransposeOmp(int const * const inputVector, int * const outputVector, int const size)
{
#pragma omp parallel for default(none) firstprivate(size) shared(inputVector, outputVector)
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            outputVector[j * size + i] = inputVector[i * size + j];
        }
    }
}


microseconds OmpRun(unsigned long size, unsigned long length)
{
    omp_set_num_threads(THREADS);

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Allocate memory for the vectors
    int *m1 = new int[length];
    int *m2 = new int[length];
    int *m3 = new int[length];


    int *m2Transposed = new int[length];
#pragma omp parallel
    {
        unsigned int seed = (unsigned int)omp_get_wtime() * omp_get_thread_num() + 1;

        // Loop through the vector, generating a random integer between 0 and 100 for each slot.
#pragma omp for nowait
        for (auto i = 0; i < length; i++)
        {
            m1[i] = rand_r(&seed) % 100;
        }

#pragma omp for
        for (auto i = 0; i < length; i++)
        {
            m2[i] = rand_r(&seed) % 100;
        }

#pragma omp single
        TransposeOmp(m2, m2Transposed, size);

        // Compute the vector addition for each element of the input vectors
#pragma omp for schedule(dynamic) collapse(1)
        for (auto i = 0; i < size; i++)
        {
            for (auto j = 0; j < size; j++)
            {
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

//    printMatrix(m1, size);
//    printMatrix(m2, size);
//    printMatrix(m3, size);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    delete[] m1;
    delete[] m2;
    delete[] m3;
    delete[] m2Transposed;

    return duration;
}


void TransposeStdThread(int const inputVector[], int outputVector[], int const size)
{
    auto worker = [=](int const threadId, int const threadCount)
    {
        for (auto i = threadId; i < size; i += threadCount)
        {
            for (auto j = 0; j < size; j++)
            {
                outputVector[j * size + i] = inputVector[i * size + j];
            }
        }
    };

    std::vector<std::thread> threads;

    for (int i = 0; i < THREADS; i++)
    {
        threads.emplace_back(worker, i, THREADS);
    }

    for (auto &thread: threads)
    {
        thread.join();
    }
}


void RandomMatrixStdThread(int const block, int const blockSize, int matrix[])
{
    unsigned int seed = time(nullptr) * (std::hash<std::thread::id>()(std::this_thread::get_id()) + 1);

    // Loop through the vector, generating a random integer between 0 and 100 for each slot.
    for (auto i = block * blockSize; i < (block + 1) * blockSize; i++)
    {
        matrix[i] = rand_r(&seed) % 100;
    }
}


void MatrixMultiplyStdThread(int const threadId, int const assignedThreads, int const matrix1[], int const matrix2Transposed[], int matrix3[], int const size)
{
    // We will assign row by row
    for (auto i = threadId; i < size; i += assignedThreads)
    {
        for (auto j = 0; j < size; j++)
        {
            int temp = 0;
            for (auto k = 0; k < size; k++)
            {
                temp += matrix1[i * size + k] * matrix2Transposed[j * size + k];
            }
            matrix3[i * size + j] = temp;
        }
    }
}


microseconds StdThreadRun(unsigned long size, unsigned long length)
{
    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Allocate memory for the vectors
    int *m1 = new int[length];
    int *m2 = new int[length];
    int *m3 = new int[length];

    {
        std::vector<std::thread> threads;

        auto blockSize = length / (THREADS / 2);

        for (int i = 0; i < THREADS / 2; i++)
        {
            threads.emplace_back(RandomMatrixStdThread, i, blockSize, m1);
        }

        for (int i = 0; i < THREADS / 2; i++)
        {
            threads.emplace_back(RandomMatrixStdThread, i, blockSize, m2);
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    // Compute the vector addition for each element of the input vectors
    {
        std::vector<std::thread> threads;

        int *m2Transposed = new int[length];
        TransposeStdThread(m2, m2Transposed, size);

        for (int i = 0; i < THREADS; i++)
        {
            threads.emplace_back(MatrixMultiplyStdThread, i, THREADS, m1, m2Transposed, m3, size);
        }

        for (auto &thread : threads)
        {
            thread.join();
        }

        delete[] m2Transposed;
    }

    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

//    printMatrix(m1, size);
//    printMatrix(m2, size);
//    printMatrix(m3, size);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    delete[] m1;
    delete[] m2;
    delete[] m3;

    return duration;
}


int main()
{
    unsigned long constexpr size = SIZE;
    unsigned long constexpr length = size * size;

    cout << "std::thread Runs" << endl;
    std::vector<microseconds> stdThreadRuns;
    for (auto i = 0; i < RUNS; i++)
    {
        stdThreadRuns.push_back(StdThreadRun(size, length));
    }
    cout << "------------------------------" << endl;

    cout << "OMP Runs" << endl;
    std::vector<microseconds> ompRuns;
    for (auto i = 0; i < RUNS; i++)
    {
        ompRuns.push_back(OmpRun(size, length));
    }


    cout << "std::thread Average: " << (std::reduce(stdThreadRuns.begin(), stdThreadRuns.end()) / stdThreadRuns.size()).count() << endl;
    cout << "OMP Average: " << (std::reduce(ompRuns.begin(), ompRuns.end()) / ompRuns.size()).count() << endl;


    return 0;
}