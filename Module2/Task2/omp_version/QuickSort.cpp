#include <random>
#include <iostream>
#include <iomanip>
#include <omp.h>


#define ARRAY_SIZE 100000
#define THREAD_COUNT 2

#define TASK_DEPTH 10


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

#pragma omp parallel for default(none) shared(array, size, distribution, randomDevice)
    for (int i = 0; i < size; i++)
    {
        array[i] = distribution(randomDevice);
    }

    std::cout << "Done Randomising" << std::endl;
}


//void prefixSum(int array[], int sums[], int start, int end)
//{
//    int length = end - start + 1;
//
//    if (length == 1)
//    {
//        sums[start] = array[start];
//    }
//    else
//    {
//        int y[length];
//        int z[length];
//        for (int i = 0; i < length; i++)
//        {
//            y[i] = 0;
//            z[i] = 0;
//        }
//
////#pragma omp for
//        for (int i = start; i <= start + (length / 2) - 1; i++)
//        {
//            y[i] = array[2*i] + array[2*i + 1];
//        }
//
//        prefixSum(y, z, start, start + (length / 2) - 1);
//
////#pragma omp for
//        for (int i = start + 1; i <= length; i++)
//        {
//            if (i == start)
//            {
//                sums[i - 1] = array[i - 1];
//            }
//            else if (i % 2 == 0)
//            {
//                sums[i - 1] = z[(i/2) - 1];
//            }
//            else
//            {
//                sums[i - 1] = z[((i-1)/2) - 1] + array[i - 1];
//            }
//        }
//    }
//}


int partition(int array[], int start, int end)
{
//    int length = end - start + 1;
//    if (length == 1)
//    {
//        return start;
//    }
//
//    int b[length];
//    int lt[length];
//    int gt[length];
//    int lt2[length];
//    int gt2[length];
//
//    for (int i = 0; i < length; i++)
//    {
//        lt[i] = 0;
//        lt2[i] = 0;
//        gt[i] = 0;
//        gt2[i] = 0;
//    }
//
//    int pivot = array[(int)std::floor((end + start) / 2)];
//
////#pragma omp parallel for
//    for (int i = 0; i < length; i++)
//    {
//        b[i] = array[start + i];
//        if (b[i] < pivot)
//        {
//            lt[i] = 1;
//        }
//        if (b[i] > pivot)
//        {
//            gt[i] = 1;
//        }
//    }
//
//    prefixSum(lt, lt2, 0, length - 1);
//    prefixSum(gt, gt2, 0, length - 1);
//
//    int k = start + lt2[length - 1];
//    array[k] = pivot;
//
////#pragma omp for
//    for (int i = 0; i < length; i++)
//    {
//        if (b[i] < pivot)
//        {
//            array[start + lt2[i] - 1] = b[i];
//        }
//        else if (b[i] > pivot)
//        {
//            array[k + gt2[i]] = b[i];
//        }
//    }
//
//    return k;

    int pivotIndex = (int)std::floor((end + start) / 2);
    int pivot = array[pivotIndex];

#ifndef NDEBUG
    std::cout << std::endl << std::endl << "Start: " << start << ", End: " << end << ", Pivot Index: " << (int)std::floor((end + start) / 2) << ", Pivot: " << pivotIndex << std::endl;
#endif

    int left = start - 1;
    int right = end + 1;

    while (true)
    {
#ifndef NDEBUG
        printIndicators(array, left, right);
#endif

        do
        {
            left++;
        } while (array[left] < pivot);

        do
        {
            right--;
        } while (array[right] > pivot);

        if (left >= right)
        {
            return right;
        }

        std::swap(array[left], array[right]);
    }
}


void quickSort(int array[], int start, int end, int level)
{
    if (start <= end)
    {
        int pivot = partition(array, start, end);

#pragma omp task if(level < TASK_DEPTH)
        quickSort(array, start, pivot - 1, level + 1);

#pragma omp task if(level < TASK_DEPTH)
        quickSort(array, pivot + 1, end, level + 1);
    }
}


int main()
{
    std::cout << "Starting" << std::endl;

    omp_set_num_threads(THREAD_COUNT);

    int array[ARRAY_SIZE];
    randomiseArray(array, ARRAY_SIZE);
    printArray(array, ARRAY_SIZE);

    auto start_time = omp_get_wtime();

#pragma omp parallel
    {
#pragma omp single
        quickSort(array, 0, ARRAY_SIZE - 1, 0);
    }

    auto end_time = omp_get_wtime();

    auto duration = end_time - start_time;

    std::cout << std::endl;
    printArray(array, ARRAY_SIZE);

    std::cout << "Execution Time: " << duration << std::endl;

    return 0;
}
