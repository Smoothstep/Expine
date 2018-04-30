#pragma once

#ifdef __CUDACC__
#define CudaKernelArgs2(grid, block) <<< grid, block >>>
#define CudaKernelArgs3(grid, block, sh_mem) <<< grid, block, sh_mem >>>
#define CudaKernelArgs4(grid, block, sh_mem, stream) <<< grid, block, sh_mem, stream >>>
#else
#define CudaKernelArgs2(grid, block)
#define CudaKernelArgs3(grid, block, sh_mem)
#define CudaKernelArgs4(grid, block, sh_mem, stream)
#endif