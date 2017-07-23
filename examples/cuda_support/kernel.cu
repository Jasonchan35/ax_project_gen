
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size);

__global__ void addKernel(int *c, const int *a, const int *b)
{
    int i = threadIdx.x;
    c[i] = a[i] + b[i];
}


/*!	\def ConvertSMVer2Cores(major, minor)
	\brief Get number of CUDA cores per multiprocessor.
	\link https://en.wikipedia.org/wiki/CUDA
	\arg[in] major GPU Architecture major version.
	\arg[in] minor GPU Architecture minor version.
	\returns 0 if GPU Architecture is unknown, or number of CUDA cores per multiprocessor.
*/
#define ConvertSMVer2Cores(major, minor) \
	(((major) == 1)? ( /* Tesla */ \
		((minor) == 0)? 8: /* G80*/ \
		((minor) == 1)? 8: /* G8x, G9x */ \
		((minor) == 2)? 8: /* GT21x */ \
		((minor) == 3)? 8: /* GT200 */ \
		0): \
	((major) == 2)? ( /* Fermi */ \
		((minor) == 0)? 32: /* GF100, GF110 */ \
		((minor) == 1)? 48: /* GF10x, FG11x */ \
		0): \
	((major) == 3)? ( /* Kepler */ \
		((minor) == 0)? 192: /* GK10x */ \
		((minor) == 2)? 192: /* GK20A */ \
		((minor) == 5)? 192: /* GK11x, GK208 */ \
		((minor) == 7)? 192: /* GK210 */ \
		0): \
	((major) == 5)? ( /* Maxwell */ \
		((minor) == 0)? 128: /* GM10X */ \
		((minor) == 2)? 128: /* GM20X */ \
		((minor) == 3)? 128: /* GM20B */ \
		0): \
	((major) == 6)? ( /* Pascal */ \
		((minor) == 0)? 64:  /* GP100 */ \
		((minor) == 1)? 128: /* GP10X */ \
		((minor) == 2)? 128: /* GP10B */ \
		0): \
	((major) == 7)? ( /* Volta */ \
		((minor) == 0)? 64:  /* GV100 */ \
		0): \
	0)

int main()
{
    const int arraySize = 5;
    const int a[arraySize] = { 1, 2, 3, 4, 5 };
    const int b[arraySize] = { 10, 20, 30, 40, 50 };
    int c[arraySize] = { 0 };

	int count = 0;
	if (cudaGetDeviceCount(&count) != cudaSuccess) return 0;
	printf("device: %d\n", count);

	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);
	printf("name: %s\n", prop.name);
	printf("compute capability: %d.%d\n", prop.major, prop.minor);
	printf("multiProcessorCount: %d (%d Cores)\n", prop.multiProcessorCount, 
		ConvertSMVer2Cores(prop.major, prop.minor) * prop.multiProcessorCount);
	printf("maxThreadsPerBlock: %d\n", prop.maxThreadsPerBlock);
	printf("maxThreadsPerMultiProcessor: %d\n", prop.maxThreadsPerMultiProcessor);
	printf("warpSize: %d\n", prop.warpSize);
	printf("maxThreadsDim: %d, %d, %d\n", prop.maxThreadsDim[0], prop.maxThreadsDim[1], prop.maxThreadsDim[2]);
	printf("totalGlobalMem: %zu MiB\n", prop.totalGlobalMem/1024/1024);
	printf("clockRate: %d MHz\n", prop.clockRate/1000);
	printf("isMultiGpuBoard: %d\n", prop.isMultiGpuBoard);
	printf("maxGridSize: %d x %d x %d\n", prop.maxGridSize[0], 
		prop.maxGridSize[1], 
		prop.maxGridSize[2]);


    // Add vectors in parallel.
    cudaError_t cudaStatus = addWithCuda(c, a, b, arraySize);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addWithCuda failed!");
        return 1;
    }

    printf("{1,2,3,4,5} + {10,20,30,40,50} = {%d,%d,%d,%d,%d}\n",
        c[0], c[1], c[2], c[3], c[4]);

    // cudaDeviceReset must be called before exiting in order for profiling and
    // tracing tools such as Nsight and Visual Profiler to show complete traces.
    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
        return 1;
    }

    return 0;
}

// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size)
{
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<1, size>>>(dev_c, dev_a, dev_b);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}
