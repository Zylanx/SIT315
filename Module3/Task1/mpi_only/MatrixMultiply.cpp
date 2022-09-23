#include <mpi.h>

// Randomise the input matrix
void randomiseMatrix(int const matrix[], int const size)
{
    for (auto i = 0; i < size; i++)
    {
        for (auto j = 0; j < size; j++)
        {
            matrix[i * size + j] = rand() % 100;
        }
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Finalize();
    return 0;
}