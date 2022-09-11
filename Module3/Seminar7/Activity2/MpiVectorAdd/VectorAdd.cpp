#include <iostream>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
#include <csignal>


using namespace std::chrono;
using namespace std;


constexpr int MASTER = 0;

constexpr int SIZE = 100000;


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

void random_vector(int vector[], unsigned long size)
{
    // Loop through the vector, generating a random integer between 0 and 100 for each slot.
    for (auto i = 0; i < size; i++)
    {
        vector[i] = rand() % 100;
    }
}

void vector_add(const int a[], const int b[], int c[], unsigned long size)
{
    for (auto i = 0; i < size; i++)
    {
        c[i] = a[i] + b[i];
    }
}

void process(const int a[], const int b[], int c[], int element_count)
{
    int a_buffer[element_count];
    int b_buffer[element_count];
    int c_buffer[element_count];

    MPI::COMM_WORLD.Scatter(a, element_count, MPI::INT, a_buffer, element_count, MPI::INT, MASTER);
    MPI::COMM_WORLD.Scatter(b, element_count, MPI::INT, b_buffer, element_count, MPI::INT, MASTER);

    vector_add(a_buffer, b_buffer, c_buffer, element_count);

    MPI::COMM_WORLD.Gather(c_buffer, element_count, MPI::INT, c, element_count, MPI::INT, MASTER);

}

void process_main(int a[], int b[], int c[], unsigned long size)
{
    // Get the number of process
    int total_worker_processes = MPI::COMM_WORLD.Get_size();
    int elements_per_process = size / total_worker_processes;
    int total_processed_elements = elements_per_process * total_worker_processes;

    if (size % total_worker_processes != 0)
    {
        vector_add(
                &a[total_processed_elements], &b[total_processed_elements], &c[total_processed_elements],
                size - total_processed_elements
        );
    }

//    for (auto i = 0; i < size % total_worker_processes; i++)
//    {
//        c[size + i] = a[size + i] + c[size + i];
//    }

    // Send the size of the element buffers to process
    MPI::COMM_WORLD.Bcast(&elements_per_process, 1, MPI::INT, MASTER);

    process(a, b, c, elements_per_process);
}

void process_worker()
{
    int element_count;
    MPI::COMM_WORLD.Bcast(&element_count, 1, MPI::INT, MASTER);

    process(nullptr, nullptr, nullptr, element_count);
}

int main(int argc, char **argv)
{
    MPI::Init(argc, argv);

    switch(MPI::COMM_WORLD.Get_rank())
    {
        case MASTER:
        {
            unsigned long size = SIZE;

            srand(time(nullptr));

            int *v1, *v2, *v3;

            // Allocate memory for the vectors
            v1 = (int *) malloc(size * sizeof(int *));
            v2 = (int *) malloc(size * sizeof(int *));
            v3 = (int *) malloc(size * sizeof(int *));

            // Randomise the input vectors
            random_vector(v1, size);
            random_vector(v2, size);

            // Store the time before the execution of the algorithm, for computing run time
            auto start = high_resolution_clock::now();

            process_main(v1, v2, v3, size);

            auto stop = high_resolution_clock::now();

            // Compute the run time of the algorithm
            auto duration = duration_cast<microseconds>(stop - start);

            printArray(v1, size);
            printArray(v2, size);
            printArray(v3, size);

            cout << "Time taken by function: "
                 << duration.count() << " microseconds" << endl;

            break;
        }
        default:
        {
            process_worker();
            break;
        }
    }

    MPI::Finalize();
    return 0;
}
