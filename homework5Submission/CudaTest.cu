#include <stdio.h>
#include <iostream>

__global__ void helloCUDA() {
    printf("Hello, CUDA!\n");
}


#include <cuda_runtime.h>
#include <iostream>

__device__ int update_flag = 0;

// Kernel to set the flag using atomic operations
__global__ void setFlag() {
    atomicExch(&update_flag, 1);
}
// Kernel to set the flag using atomic operations
__global__ void resetFlag() {
    atomicExch(&update_flag, 0);
}

// Kernel to check and print the value of the flag
__global__ void checkFlag() {
    printf("Flag value in device: %d\n", update_flag);
}

int main() {
    helloCUDA<<<1, 1>>>(); // Launch kernel with 1 block and 10 threads
    cudaDeviceSynchronize(); // Wait for the GPU to finish

    int deviceId = 0; // Assuming you want to query the first GPU
    cudaDeviceProp deviceProp;
    cudaGetDeviceProperties(&deviceProp, deviceId);

    std::cout << "Device Name: " << deviceProp.name << std::endl;
    std::cout << "Device ID: " << deviceId << std::endl;

    std::cout << "Number of SMs: " << deviceProp.multiProcessorCount << std::endl;
    std::cout << "Max blocks per SM: " << deviceProp.maxBlocksPerMultiProcessor << std::endl;
    std::cout << "Max threads per SM: " << deviceProp.maxThreadsPerMultiProcessor << std::endl;


    // Allocate memory for the device flag
    int* d_update_flag;
    cudaMalloc(&d_update_flag, sizeof(int));

    // Initialize the flag to 0 using cudaMemset
    cudaMemset(d_update_flag, 0, sizeof(int));
    std::cout << "Initial flag value set to 0 via cudaMemset" << std::endl;

    // Launch kernel to set the flag
    setFlag<<<1, 1>>>();
    cudaDeviceSynchronize();

    // Launch kernel to check the flag
    checkFlag<<<1, 1>>>();
    cudaDeviceSynchronize();

    // Reset the flag using cudaMemset
    cudaMemset(d_update_flag, 0, sizeof(int));
    std::cout << "Flag reset to 0 via cudaMemset" << std::endl;
    resetFlag<<<1,1>>>();

    // Launch kernel to check the flag again
    checkFlag<<<1, 1>>>();
    cudaDeviceSynchronize();

    // Free the allocated memory
    cudaFree(d_update_flag);

    return 0;
}
