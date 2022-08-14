#include <iostream>
#include <cstdlib>
#include <ctime>
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


// Transposes a matrix into another pointer.
void transpose(int const inputMatrix[], int outputMatrix[], int const size)
{
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            outputMatrix[j * size + i] = inputMatrix[i * size + j];
        }
    }
}


int main()
{
    // Set up the matrix column size, and total length of the storage arrays
    unsigned long constexpr size = 512;
    unsigned long constexpr length = size * size;

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
    transpose(m2, m2Transposed, size);

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

    // Print the results of the algorithm.
    printMatrix(m1, size);
    printMatrix(m2, size);
    printMatrix(m3, size);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}