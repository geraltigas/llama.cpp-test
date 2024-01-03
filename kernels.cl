__kernel void compare_matrices(__global const float* matrixA, __global const float* matrixB, __global bool* result, const int rows, const int cols) {
    int row = get_global_id(0);
    int col = get_global_id(1);

    if (row < rows && col < cols) {
        int index = row * cols + col;
        if (matrixA[index] != matrixB[index]) {
            *result = false;
        }
    }
}