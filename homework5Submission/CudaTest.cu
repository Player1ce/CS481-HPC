#include <stdio.h>
#include <iostream>

__global__ void helloCUDA() {
    printf("Hello, CUDA!\n");
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

    return 0;
}
