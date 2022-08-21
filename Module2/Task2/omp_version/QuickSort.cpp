#include <random>
#include <iostream>
#include <iomanip>
#include <omp.h>


#define ARRAY_SIZE 100000
#define THREAD_COUNT 4

#define TASK_DEPTH 5


// Print out the given array.
void printArray(int array[], int size)
{
    // Sets the output to be left justified
    std::cout << std::left;

    // Loop through the array, printing out each element
    for (int i = 0; i < size; i++) {
        // Sets the field width to 4 before printing an element
        std::cout << std::setw(4) << array[i];
    }

    std::cout << std::endl;
}


#ifndef NDEBUG
// Print the array with indicators of the two passed in indexes under it.
void printIndicators(int array[], int first, int second)
{
    // Work out which indicator is to the left, and which is to the right
    // clamping them to the valid indexes
    int left = std::max(std::min(first, second), 0);
    int right = std::min(std::max(first, second), ARRAY_SIZE - 1);

    printArray(array, ARRAY_SIZE);

    // Now print the left indicator, prepending enough spaces to fill the empty fields
    std::cout << std::string(left * 4, ' ') << std::setw(4) << "^";

    // If the indicators aren't in the same index, then print out the next indicator.
    if (right != left)
    {
        std::cout << std::string(((right - 1) * 4) - (left * 4), ' ') << std::setw(4) << "^";
    }

    std::cout << std::endl;
}
#endif


// Randomise the elements of the given array.
void randomiseArray(int array[], int size)
{
    // Instantiate an instance of a true random number generator, and a uniform distribution.
    std::random_device randomDevice;
    std::uniform_int_distribution<int> distribution(0, 100);

    // Loop through all its elements in parallel, randomising them
#pragma omp parallel for default(none) shared(array, size, distribution, randomDevice)
    for (int i = 0; i < size; i++)
    {
        array[i] = distribution(randomDevice);
    }
}


// Partitions the array into two based on a pivot, returning the pivot location.
// Elements less than the pivot in one partition and elements greater than the pivot in the other.
int partition(int array[], int start, int end)
{
    // Select a pivot midway into the array
    int pivot = array[(int)std::floor((end + start) / 2)];

#ifndef NDEBUG
    // Print some debug stats for the initial state
    std::cout << std::endl << std::endl << "Start: " << start << ", End: " << end << ", Pivot Index: " << (int)std::floor((end + start) / 2) << ", Pivot: " << pivot << std::endl;
#endif

    // Initialise a left and right index
    int *left = &array[start] - 1;
    int *right = &array[end] + 1;

    // Recursively loop through the array, swapping elements greater than the pivot that are to the left of it into
    // the right partition with elements less than it to the right of it.
    while (true)
    {
#ifndef NDEBUG
        // Print out some debug info for each iteration
        printIndicators(array, left - array, right - array);
#endif

        // Move the left index till it finds an element greater than the pivot
        do
        {
            left++;
        } while (*left < pivot);

        // Move the right index till it finds an element less than the pivot
        do
        {
            right--;
        } while (*right > pivot);

        if (left >= right)
        {
            return right - array;
        }

        std::swap(array[left - array], array[right - array]);
    }
}


// Sorts the given array using the quicksort method recursively.
// Uses task based parallelism and recursive decomposition to get some potential speed-up.
void quickSort(int array[], int start, int end, int level)
{
    // if the start and end are swapped, then this is an invalid sublist
    if (start >= end)
    {
        return;
    }

    // Partition the sublist, saving the pivot
    int pivot = partition(array, start, end);

    // Now pivot the partitions on either side of the pivot
    // The if clause with the level ensures that there is a cap on the number of tasks that can be defered at once.
    // Once the if clause is false, the task will be executed immediately.
#pragma omp task default(none) firstprivate(array, pivot) shared(start, end, level) if(level < TASK_DEPTH)
    quickSort(array, start, pivot, level + 1);

#pragma omp task default(none) firstprivate(array, pivot) shared(start, end, level) if(level < TASK_DEPTH)
    quickSort(array, pivot + 1, end, level + 1);
}


int main()
{
    // Set the number of threads available to OpenMP
    omp_set_num_threads(THREAD_COUNT);

    // Initialise an array with the given size and randomise its elements.
    int array[ARRAY_SIZE];
    randomiseArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    // Store the start time
    auto start_time = omp_get_wtime();

    // Start the quickSort in parallel. Make sure only one thread makes the initial call.
#pragma omp parallel default(none) firstprivate(array)
    {
#pragma omp single
        quickSort(array, 0, ARRAY_SIZE - 1, 0);
    }

    // Store the end time
    auto end_time = omp_get_wtime();

    // Calculate the duration
    auto duration = (end_time - start_time) * 100000;

    // Print the sorted array
    std::cout << std::endl;
    printArray(array, ARRAY_SIZE);

    // Print the execution time
    std::cout << "Execution Time: " << duration << std::endl;

    return 0;
}
