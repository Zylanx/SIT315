#include <random>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <algorithm>


#define ARRAY_SIZE 30
#define THREAD_COUNT 8

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
void printIndicators(int array[], int size, int first, int second)
{
    // Work out which indicator is to the left, and which is to the right
    // clamping them to the valid indexes
    int left = std::max(std::min(first, second), 0);
    int right = std::min(std::max(first, second), ARRAY_SIZE - 1);

    printArray(array, size);

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


void parallelPrefixSum(int array[], int result[], int size)
{
    result[0] = array[0];

    if (size > 1)
    {
        int y[size / 2];
        int z[size / 2];
        for (auto i = 0; i < size / 2; i++)
        {
            y[i] = array[2 * i] + array[(2 * i) + 1];
        }

        parallelPrefixSum(y, z, size / 2);

        for (auto i = 1; i < size; i++)
        {
            if (i % 2 == 1)
            {
                result[i] = z[(i - 1) / 2];
            }
            else
            {
                result[i] = z[(i - 2) / 2] + array[i];
            }
        }
    }
}


// Partitions the array into two based on a pivot, returning the pivot location.
// Elements less than the pivot in one partition and elements greater than the pivot in the other.
int partition(int array[], int size)
{
    // Select a pivot midway into the array
    long pivotIndex = (long)std::floor((size - 1) / 2);
    int pivot = array[pivotIndex];

#ifndef NDEBUG
    // Print some debug stats for the initial state
    std::cout << std::endl << std::endl << "Start: " << 0 << ", End: " << size << ", Pivot Index: " << (int)std::floor(size / 2) << ", Pivot: " << pivot << std::endl;
#endif

    if (size == 1)
    {
        return 0;
    }

    int B[size];
    int lt[size];
    int gt[size];
    int lt2[size];
    int gt2[size];

    for (auto i = 0; i < size; i++)
    {
        B[i] = array[i];
        lt[i] = B[i] < pivot ? 1 : 0;
        gt[i] = B[i] >= pivot && i != pivotIndex ? 1 : 0;
    }

    parallelPrefixSum(lt, lt2, size);
    parallelPrefixSum(gt, gt2, size);

    auto k = lt2[size - 1];
    array[k] = pivot;

    for (auto i = 0; i < size; i++)
    {
        if (i == pivotIndex)
        {
            continue;
        }

        if (B[i] < pivot)
        {
            array[lt2[i] - 1] = B[i];
        }
        else if (B[i] >= pivot)
        {
            array[k + gt2[i]] = B[i];
        }
    }

    return k;
}


// Sorts the given array using the quicksort method recursively.
// Uses task based parallelism and recursive decomposition to get some potential speed-up.
void quickSort(int array[], int size, int level)
{
    // if the start and end are swapped, then this is an invalid sublist
    if (size <= 1)
    {
        return;
    }

    // Partition the sublist, saving the pivot
    int pivot = partition(array, size);

    // Now pivot the partitions on either side of the pivot
    // The if clause with the level ensures that there is a cap on the number of tasks that can be deferred at once.
    // Once the if clause is false, the task will be executed immediately.
//#pragma omp task default(none) firstprivate(array, pivot) shared(size, level) if(level < TASK_DEPTH)
    quickSort(array, pivot, level + 1);

//#pragma omp task default(none) firstprivate(array, pivot) shared(size, level) if(level < TASK_DEPTH)
    quickSort((&array[pivot]) + 1, size - (pivot + 1), level + 1);
}


int main()
{
    // Set the number of threads available to OpenMP
    omp_set_num_threads(THREAD_COUNT);

    // Initialise an array with the given size and randomise its elements.
//    int array[ARRAY_SIZE] = {66, 87, 3, 24, 56, 4, 56, 41, 7, 52, 32, 0, 4, 85, 23, 11, 70, 83, 68, 58, 64, 43, 15, 27, 84, 64, 5, 85, 23, 59, 26, 98, 10, 63, 31, 85, 24, 0, 34, 36, 28, 27, 93, 26, 22, 9, 91, 24, 26, 14, 75, 61, 22, 99, 60, 18, 41, 77, 87, 55, 82, 52, 19, 34, 83, 75, 85, 51, 63, 72, 11, 86, 37, 26, 20, 91, 31, 1, 20, 11, 70, 71, 6, 82, 70, 26, 58, 20, 9, 91, 46, 17, 66, 16, 82, 14, 7, 1, 62, 63};
    int array[ARRAY_SIZE] = {58, 100, 45, 82, 33, 17, 20, 37, 15, 48, 12, 83, 31, 12, 85, 78, 69, 87, 97, 97, 66, 7, 95, 76, 54, 50, 57, 54, 57, 51};
//    randomiseArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    int array2[ARRAY_SIZE];
    for (auto i = 0; i < ARRAY_SIZE; i++)
    {
        array2[i] = array[i];
    }
    std::sort(array2, &array2[ARRAY_SIZE]);
//
//    quickSort(array2, ARRAY_SIZE, 0);
//    std::cout << std::endl << "Is Sorted? " << (std::is_sorted(array2, &array2[ARRAY_SIZE - 1]) ? "True" : "False") << std::endl << std::flush;

    // Store the start time
    auto start_time = omp_get_wtime();

    // Start the quickSort in parallel. Make sure only one thread makes the initial call.
//#pragma omp parallel default(none) shared(array)
//    {
//#pragma omp single
        quickSort(array, ARRAY_SIZE, 0);
//    }

    // Store the end time
    auto end_time = omp_get_wtime();

    // Calculate the duration
    auto duration = (end_time - start_time) * 100000;

    // Print the sorted array
    std::cout << std::endl;
    printArray(array, ARRAY_SIZE);

    std::cout << std::endl << "Is Sorted? " << (std::is_sorted(array, &array[ARRAY_SIZE - 1]) ? "True" : "False") << std::endl;

    if (std::equal(array, &array[ARRAY_SIZE], array2, &array2[ARRAY_SIZE]))
    {
        std::cout << std::endl << "Correctly Sorted" << std::endl;
    }
    else
    {
        std::cout << std::endl << "Incorrectly Sorted" << std::endl;
    }

    // Print the execution time
    std::cout << "Execution Time: " << duration << std::endl;

    return 0;
}
