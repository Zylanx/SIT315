#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <omp.h>


using namespace std::chrono;
using namespace std;


// Helper function to print arrays
void printArray(int *vector, int size)
{

    cout << vector[0];
    for (int i = 1; i < size; i++)
    {
        cout << ", " << vector[i];

        if( i % 20 == 0){
            cout <<"\n";
        }

    }
    cout << "\n";
    cout << "\n***************************************************************************************\n";
}


int main()
{
    unsigned long size = 100000000;

    int *v1, *v2, *v3;

    // Store the time before the execution of the algorithm, for computing run time
    auto start = high_resolution_clock::now();

    // Allocate memory for the vectors
    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

    omp_set_num_threads(omp_get_max_threads());

    // Randomise the input arrays in parallel.
    // Both can be done at the same time, so ensure the for loops
    // have the nowait clause to remove the barriers between them.
#pragma omp parallel default(none) shared(size, v1, v2)
    {
        // Create a private initial seed for each thread
        unsigned int seed = time(nullptr) * (omp_get_thread_num() + 1);

        // Randomise Vector 1
#pragma omp for nowait
        for (int i = 0; i < size; i++)
        {
            v1[i] = rand_r(&seed) % 100;
        }

        // Randomise Vector 2
#pragma omp for nowait
        for (int i = 0; i < size; i++)
        {
            v2[i] = rand_r(&seed) % 100;
        }
    }

    // Do the vector addition in parallel
#pragma omp parallel for default(none) shared(size, v1, v2, v3)
    {
        // Compute the vector addition for each element of the input vectors
        for (int i = 0; i < size; i++)
        {
            v3[i] = v1[i] + v2[i];
        }
    }

    // Calculate the total using a reduction clause
    int total;
#pragma omp parallel default(none) shared(size, v3, total)
    {
#pragma omp for reduction(+:total)
        for (int i = 0; i < size; i++)
        {
            total += v3[i];
        }
    };
    cout << "Total 1: " << total << endl;

    // Calculate the total using a parallel for loop then a
    // critical section to bring all the data together.
    int total2;
#pragma omp parallel default(none) shared(total2, size, v3)
    {
        int privateTotal;

#pragma omp for
        for (int i = 0; i < size; i++)
        {
            privateTotal += v3[i];
        }

#pragma omp critical
        {
            total2 += privateTotal;
        };
    }
    cout << "Total 2: " << total2 << endl;

    // Store the time after the execution of the algorithm
    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

    // Display the first 100 elements of the vectors
    cout <<"v1: \n\n  ";
    printArray(v1, size < 100 ? size : 100);
    cout <<"v2: \n\n  ";
    printArray(v2, size < 100 ? size : 100);
    cout <<"v3: \n\n  ";
    printArray(v3, size < 100 ? size : 100);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}