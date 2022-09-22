// Calculates the magnitude of the square of a vector.
// Each element of the vector is a work unit that is spread across the devices elements.
__kernel void add_vectors(const int size,
                      __global int* v1, __global int* v2) {
    
    // Thread identifiers
    const int globalIndex = get_global_id(0);   
 
    //uncomment to see the index each PE works on
    //printf("Kernel process index :(%d)\n ", globalIndex);

    v1[globalIndex] = v1[globalIndex] + v2[globalIndex];
}
