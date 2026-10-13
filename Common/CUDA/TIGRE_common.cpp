#if defined(IS_FOR_PYTIGRE)
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "TIGRE_common.hpp"
void mexPrintf(const char* format, ...) {
    PRINT_HERE("");
    va_list argpointer;
    va_start(argpointer, format);
    vprintf(format, argpointer);
    va_end(argpointer);
}
void mexErrMsgIdAndTxt(const char* pcTag, const char* pcMsg) {
    PRINT_HERE("%s %s\n", pcTag, pcMsg);
    exit(1);
}
void mexWarnMsgIdAndTxt(const char* pcTag, const char* pcMsg) {
    PRINT_HERE("%s %s\n", pcTag, pcMsg);
}
#endif  // IS_FOR_PYTIGRE

#include <cuda_runtime_api.h>
#include <cuda.h> 
#include <cstring>
#include <sstream>

void cudaCheckErrors(const char* msg, const char * filename, int lineno)
{
    do {
        cudaError_t __err = cudaGetLastError();
        if (__err != cudaSuccess) {         
                mexPrintf("%s \n",msg);
                cudaDeviceReset();

                std::ostringstream msgLocation;
                msgLocation << filename << ":" << lineno << " @ " << "cudaCheckErrors";  

                mexErrMsgIdAndTxt(msgLocation.str().c_str(),cudaGetErrorString(__err));
        }
    } while (0);
};

//______________________________________________________________________________
//
//      Function:       checkFreeMemory
//
//      Description:    check available memory on devices
//______________________________________________________________________________
void checkFreeMemory(const GpuIds& gpuids, size_t *mem_GPU_global, const char * filename, int lineno){
    size_t memfree;
    size_t memtotal;
    int deviceCount = gpuids.GetLength();
    for (int dev = 0; dev < deviceCount; dev++){
        cudaSetDevice(gpuids[dev]);
        cudaMemGetInfo(&memfree,&memtotal);
        if(dev==0) *mem_GPU_global=memfree;
        if(memfree<memtotal/2){
            std::ostringstream msgLocation;
            msgLocation << filename << ":" << lineno << " @ " << "checkFreeMemory";
            mexErrMsgIdAndTxt(msgLocation.str().c_str(),"One (or more) of your GPUs is being heavily used by another program (possibly graphics-based).\n Free the GPU to run TIGRE\n");
        }
        cudaCheckErrors("Check mem error");
        *mem_GPU_global=(memfree<*mem_GPU_global)?memfree:*mem_GPU_global;
    }
    *mem_GPU_global=(size_t)((double)*mem_GPU_global*0.95);
    
    //*mem_GPU_global= insert your known number here, in bytes.
}

//______________________________________________________________________________
//
//      Function:       freeGeoArray
//
//      Description:    Frees the memory from the geometry array for multiGPU.
//______________________________________________________________________________
void freeGeoArray(unsigned int splits,Geometry* geoArray){
    for(unsigned int sp=0;sp<splits;sp++){
        free(geoArray[sp].offOrigZ);
    }
    free(geoArray);
}



