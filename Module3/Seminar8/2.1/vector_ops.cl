// Calculates the magnitude of the square of a vector.
// Each element of the vector is a work unit that is spread across the devices elements.
__kernel void square_magnitude(const int size,
                      __global int* v) {
    
    // Thread identifiers
    const int globalIndex = get_global_id(0);   
 
    //uncomment to see the index each PE works on
    //printf("Kernel process index :(%d)\n ", globalIndex);

    v[globalIndex] = v[globalIndex] * v[globalIndex];
}
