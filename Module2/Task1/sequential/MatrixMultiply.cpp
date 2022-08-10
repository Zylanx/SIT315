#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>


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

    cout << "\n";
    cout << "\n***************************************************************************************\n";
}


// TODO: Redo the comments from vector to matrix

void randomMatrix(int matrix[], unsigned int const length)
{
    // Loop through the vector, generating a random integer between 0 and 100 for each slot.
    for (int i = 0; i < length; i++)
    {
        matrix[i] = rand() % 100;
    }
}

void TransposeOmp(int const inputVector[], int outputVector[], int const size)
{
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            outputVector[j * size + i] = inputVector[i * size + j];
        }
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
    randomMatrix(m2, totalSize);

    int *m2Transposed = new int[size * size];
    TransposeOmp(m2, m2Transposed, size);

    // Compute the vector addition for each element of the input vectors
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            int temp = 0;
            for (int k = 0; k < size; k++)
            {
                temp += m1[i * size + k] * m2Transposed[j * size + k];
            }
            m3[i * size + j] = temp;
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

    return 0;
}