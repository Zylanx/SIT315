#include <iostream>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>


#define PARTITION_COUNT 8


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


// Fill a vector with random numbers between 0 and 100 with multi-threading.
void randomVector(int vector[], std::vector<std::thread> &threads, int threadCount, int partitionSize)
{
    // Worker thread which generates random numbers and puts them in the vector.
    // Inserts them into a given range.
    auto randomPartition = [=](int start, int end)
    {
        // Initialise a seed based on the time and threads id
        unsigned int seed = time(nullptr) * (std::hash<std::thread::id>()(std::this_thread::get_id()) + 1);

        // Loop through the assigned range, generating a random integer between 0 and 100.
        for (unsigned long i = start; i < end; i++)
        {
            int random = rand_r(&seed) % 100; // We must use rand_r to make this thread-safe. Massive slowdown otherwise
            vector[i] = random;
        }
    };

    // split the vector across the assigned threads
    for (int partition = 0; partition < threadCount; partition++)
    {
        // Create the thread and add it to the thread list, for later joining.
        threads.emplace_back(randomPartition, partition * partitionSize, (partition + 1) * partitionSize);
    }
}


// Fill multiple vectors with random numbers
void randomVectors(std::vector<int*> const &vectors, int partitionSize)
{
    partitionSize *= 2;


    // Fill each vector with random numbers, splitting the threads between them.
    std::vector<std::thread> threads;
    for (auto &vector : vectors)
    {
        randomVector(vector, threads, PARTITION_COUNT / vectors.size(), partitionSize);
    }

    // Wait for all threads to finish.
    for (auto &thread : threads)
    {
        thread.join();
    }
}


// Add two input vectors together and put the result in the output vector.
void addVector(const int leftVector[], const int rightVector[], int result[], int partitionSize)
{
    // Worker thread which adds a range of values in two vectors and puts the result in the output vector.
    auto addPartition = [=](int start, int end)
    {
        for (int i = start; i < end; i++)
        {
            result[i] = leftVector[i] + rightVector[i];
        }
    };

    // Split the vector up across the threads and run them
    std::vector<std::thread> threads;
    for (int partition = 0; partition < PARTITION_COUNT; partition++)
    {
        threads.emplace_back(addPartition, partition * partitionSize, (partition + 1) * partitionSize);
    }

    // Finally join on all the workers
    for (auto & thread : threads)
    {
        thread.join();
    }
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

    // Store the size of partitions for the defined partition count
    int partitionSize = size / PARTITION_COUNT;

    // Randomise the input vectors data.
    randomVectors({v1, v2}, partitionSize);

    // Compute the vector addition for each element of the input vectors
    addVector(v1, v2, v3, partitionSize);

    // Store the time after the execution of the algorithm
    auto stop = high_resolution_clock::now();

    // Compute the run time of the algorithm
    auto duration = duration_cast<microseconds>(stop - start);

    // write the first 100 elements of the vectors
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