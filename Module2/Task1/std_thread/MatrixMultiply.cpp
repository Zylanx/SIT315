#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>


using namespace std::chrono;
using namespace std;


#define THREAD_COUNT 8

// Helper function to print arrays
void PrintMatrix(int const matrix[], int const size)
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



// TODO: Redo the comments from vector to matrix

void Transpose(int const inputVector[], int outputVector[], int const size)
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

    for (int i = 0; i < THREAD_COUNT; i++)
    {
        threads.emplace_back(worker, i, THREAD_COUNT);
    }

    for (auto &thread: threads)
    {
        thread.join();
    }
}

void RandomMatrix(int const block, int const blockSize, int matrix[])
{
    unsigned int seed = time(nullptr) * (std::hash<std::thread::id>()(std::this_thread::get_id()) + 1);

    // Loop through the vector, generating a random integer between 0 and 100 for each slot.
    for (auto i = block * blockSize; i < (block + 1) * blockSize; i++)
    {
        matrix[i] = rand_r(&seed) % 100;
    }
}


// TODO: We will use
void MatrixMultiply(int const threadId, int const assignedThreads, int const matrix1[], int const matrix2Transposed[], int matrix3[], int const size)
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


int main()
{
    unsigned long constexpr size = 512;
    unsigned long constexpr totalSize = size * size;

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Allocate memory for the vectors
    int *m1 = new int[totalSize];
    int *m2 = new int[totalSize];
    int *m3 = new int[totalSize];

    {
        std::vector<std::thread> threads;

        auto blockSize = totalSize / (THREAD_COUNT / 2);

        for (int i = 0; i < THREAD_COUNT / 2; i++)
        {
            threads.emplace_back(RandomMatrix, i, blockSize, m1);
        }

        for (int i = 0; i < THREAD_COUNT / 2; i++)
        {
            threads.emplace_back(RandomMatrix, i, blockSize, m2);
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }

    // Compute the vector addition for each element of the input vectors
    {
        std::vector<std::thread> threads;

        int *m2Transposed = new int[totalSize];
        Transpose(m2, m2Transposed, size);

        for (int i = 0; i < THREAD_COUNT; i++)
        {
            threads.emplace_back(MatrixMultiply, i, THREAD_COUNT, m1, m2Transposed, m3, size);
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