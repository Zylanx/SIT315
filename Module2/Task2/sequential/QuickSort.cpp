#include <random>
#include <iostream>
#include <iomanip>
#include <omp.h>


#define ARRAY_SIZE 100000


void printArray(int array[], int size)
{
    std::cout << std::left;

    for (int i = 0; i < size; i++) {
        std::cout << std::setw(4) << array[i];
    }
    std::cout << std::endl;
}


#ifndef NDEBUG
void printIndicators(int array[], int first, int second)
{
    int left = std::max(std::min(first, second), 0);
    int right = std::min(std::max(first, second), ARRAY_SIZE - 1);

    printArray(array, ARRAY_SIZE);
    std::cout << std::string(left * 4, ' ') << std::setw(4) << "^";

    if (right != left)
    {
        std::cout << std::string(((right - 1) * 4) - (left * 4), ' ') << std::setw(4) << "^" << std::endl;
    }
    else
    {
        std::cout << std::endl;
    }
}
#endif


void randomiseArray(int array[], int size)
{
    std::random_device randomDevice;
    std::uniform_int_distribution<int> distribution(0, 100);

    std::cout << "Randomising" << std::endl;

    for (int i = 0; i < size; i++)
    {
        array[i] = distribution(randomDevice);
    }

    std::cout << "Done Randomising" << std::endl;
}


int partition(int array[], int start, int end)
{
    int pivot = array[(int)std::floor((end + start) / 2)];

#ifndef NDEBUG
    std::cout << std::endl << std::endl << "Start: " << start << ", End: " << end << ", Pivot Index: " << (int)std::floor((end + start) / 2) << ", Pivot: " << pivot << std::endl;
#endif

    start--;
    end++;

    while (true)
    {
#ifndef NDEBUG
        printIndicators(array, start, end);
#endif

        do
        {
            start++;
        } while (array[start] < pivot);

        do
        {
            end--;
        } while (array[end] > pivot);

        if (start >= end)
        {
            return end;
        }

        std::swap(array[start], array[end]);
    }
}


void quickSort(int array[], int start, int end)
{
    if (start < end)
    {
        int pivot = partition(array, start, end);

        quickSort(array, start, pivot);
        quickSort(array, pivot + 1, end);
    }
}


int main()
{
    std::cout << "starting" << std::endl;

    int array[ARRAY_SIZE];
    randomiseArray(array, ARRAY_SIZE);

    printArray(array, ARRAY_SIZE);

    auto start_time = omp_get_wtime();

    quickSort(array, 0, ARRAY_SIZE - 1);

    auto end_time = omp_get_wtime();
    auto duration = end_time - start_time;

    std::cout << std::endl;
    printArray(array, ARRAY_SIZE);

    std::cout << "Execution Time: " << duration << std::endl;

    return 0;
}
