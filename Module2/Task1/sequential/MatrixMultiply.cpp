#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>


using namespace std::chrono;
using namespace std;


// TODO: Redo the comments from vector to matrix

void randomMatrix(int matrix[], unsigned int const length)
{
    // Loop through the vector, generating a random integer between 0 and 100 for each slot.
    for (int i = 0; i < length; i++)
    {
        matrix[i] = rand() % 100;
    }
}


int main()
{
    unsigned long constexpr size = 512;
    unsigned long constexpr totalSize = size * size;

    srand(time(nullptr));

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Allocate memory for the vectors
    int *m1 = (int *)malloc(sizeof(int *) * size * size);
    int *m2 = (int *)malloc(sizeof(int *) * size * size);
    int *m3 = (int *)malloc(sizeof(int *) * size * size);

    randomMatrix(m1, totalSize);
    randomMatrix(m1, totalSize);

    // Compute the vector addition for each element of the input vectors
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            m3[i * size + j] = 0;
            for (int k = 0; k < size; k++)
            {
                m3[i * size + j] += m1[i * size + k] * m2[j * size + k];
            }
        }
    }

    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}