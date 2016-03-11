/* compile

   export PKG_CONFIG_PATH=/opt/ocl/babl/install/lib/pkgconfig:/opt/ocl/GEGL-OpenCL/install/lib/pkgconfig:/opt/ocl/GEGL_wrapper/install/lib/pkgconfig
   LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/ocl/GEGL-OpenCL/install/lib:/opt/ocl/babl/install/lib:/opt/ocl/babl/install/lib/babl-0.1
   PATH=/opt/ocl/GEGL-OpenCL/bin:$PATH
   g++ -O3 GEGL_invert_gamma_ocl.c `pkg-config --libs --cflags gegl-wrapper`  `pkg-config --libs --cflags gegl-0.3` -lOpenCL -o GEGL_invert_gamma_ocl && ./GEGL_invert_gamma_ocl


no opencl:
   device: AMD Phenom(tm) II X4 B93 Processor
   Execution time is: 2.422 ms


number of platforms: 2
number of devices: 1
choose device: AMD Phenom(tm) II X4 B93 Processor
OpenCl write execution time is: 2.353 ms 
OpenCl read execution time is: 5.210 ms 
OpenCl kernel execution time is: 2.789 ms 
OpenCl total execution time is: 10.353 ms 
Overall execution time is: 1214.332 ms 


number of platforms: 2
number of devices: 0
  skip this device (wrong type)
number of devices: 1
choose device: GeForce GTX 650 Ti
OpenCl write execution time is: 2.059 ms 
OpenCl read execution time is: 1.344 ms 
OpenCl kernel execution time is: 0.080 ms 
OpenCl total execution time is: 3.483 ms 
Overall execution time is: 1187.273 ms 

*/   



#include "GEGLwrapper.h"
#include <gegl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

//#define DEVICE_TYPE CL_DEVICE_TYPE_CPU
#define DEVICE_TYPE CL_DEVICE_TYPE_GPU

int main(int argc, char* argv[]) {
    /*
       clock_t start, end;
       double cpu_time_used;
       struct GEGLclass *c = newGEGLclass(argc, argv);
       if (c == NULL)
       return 0;

       set_colorformat(c, "R'G'B'A float");
       float *in, *out;
       get_in_out(c, &in, &out);
       long pixels = get_pixelcount(c);
       start = clock();
       while (pixels--) {
       out[0] = 1.0 - in[0];
       out[1] = 1.0 - in[1];
       out[2] = 1.0 - in[2];
       out[3] = in[3];

       in += 4;
       out+= 4;
       }
       end = clock();

       set_output(c);
       deleteGEGLclass(c);

       cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
       printf("Execution time is: %0.3f ms \n", cpu_time_used * 1000);
       */



    clock_t start, end;
    double cpu_time_used;
    struct GEGLclass *c = newGEGLclass(argc, argv);
    if (c == NULL)
        return 0;

    set_colorformat(c, "R'G'B'A float");
    float *in, *out;
    start = clock();
    get_in_out(c, &in, &out);
    long pixels = get_pixelcount(c);
    // keep the .c beginning


    int err;
    cl_int status;
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(numPlatforms == 0) {
        perror("Couldn't identify a platform");
        exit(1);
    } 
    printf("number of platforms: %d\n", numPlatforms);
    platforms = (cl_platform_id*) malloc(numPlatforms * sizeof(cl_platform_id));
    status = clGetPlatformIDs(numPlatforms, platforms, NULL);

    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;
    int i;
    for(i = 0; i < numPlatforms; i++) {
        status = clGetDeviceIDs(platforms[i], DEVICE_TYPE, 0, NULL,
                                &numDevices);
        devices = (cl_device_id*) malloc(numDevices*sizeof(cl_device_id));
        status = clGetDeviceIDs(platforms[i], DEVICE_TYPE, numDevices,
                                devices, NULL);
        printf("number of devices: %d\n", numDevices);
        if(numDevices == 0) {
            printf("  skip this device (wrong type)\n");
            continue;
        }
        // we found a device
        break;
    }
    if(i == numPlatforms) {
        perror("Couldn't identify a device");
        exit(1);
    } 

    // print chosen device name
    char* value;
    size_t valueSize;
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &valueSize);
    value = (char*) malloc(valueSize);
    clGetDeviceInfo(devices[0], CL_DEVICE_NAME, valueSize, value, NULL);
    printf("choose device: %s\n", value);
    free(value);

    cl_context context = NULL;
    context = clCreateContext(NULL, numDevices, devices, NULL, NULL, &status);
    cl_command_queue cmdQueue;
    cmdQueue = clCreateCommandQueue(context, devices[0],
                                    CL_QUEUE_PROFILING_ENABLE, &status);

    FILE *fp;
    const char fileName[] = "invert-gamma.cl";
    size_t source_size;
    char *source_str;
    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");
        exit(1);
    }
    source_str = (char *) malloc(MAX_SOURCE_SIZE);
    source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    cl_program program = clCreateProgramWithSource(context, 
                                                   1, 
                                                   (const char**) &source_str, 
                                                   (const size_t *) &source_size, 
                                                   &status);
    status = clBuildProgram(program, numDevices, devices, NULL, NULL, NULL);
    cl_kernel kernel = NULL;
    kernel = clCreateKernel(program, "cl_invert_gamma", &status);


    cl_mem bufferIn, bufferOut;

    bufferIn = clCreateBuffer(context, CL_MEM_READ_ONLY, 
                              pixels*4*sizeof(float), NULL, &status);
    if (status != CL_SUCCESS) {
        printf("clCreateBuffer() Error %d: Failed to create bufferIn!\n", status);
        return 0;
    }


    bufferOut = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                               pixels*4*sizeof(float), NULL, &status);
    if (status != CL_SUCCESS) {
        printf("clCreateBuffer() Error %d: Failed to create bufferOut!\n", status);
        return 0;
    }


    // profiling
    cl_event write_in;
    cl_event kernel_ev;
    cl_event read_out;

    status = clEnqueueWriteBuffer(cmdQueue, bufferIn, CL_FALSE, 0,
                                  pixels*4*sizeof(float), in, 0, NULL, &write_in);
    if (status != CL_SUCCESS)
    {
        printf("clEnqueueWriteBuffer() Error %d: Failed to write bufferIn!\n", status);
        return 0;
    }

    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferIn);
    status |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferOut);
    if (status != CL_SUCCESS)
    {
        printf("clEnqueueclEnqueueWriteBufferWriteBuffer() Error \n");
        return 0;
    }


    size_t globalWorkSize[1];
    globalWorkSize[0] = pixels;


    // start computing
    status = clEnqueueNDRangeKernel(cmdQueue, kernel, 1, NULL, globalWorkSize,
                                    NULL, 0, NULL, &kernel_ev);
    if (status != CL_SUCCESS) {
        printf("clEnqueueNDRangeKernel() Error %d: Failed to execute kernel!\n", status);
        return 0;
    }

    // get results    
    status = clEnqueueReadBuffer(cmdQueue, bufferOut, CL_TRUE, 0,
                                 pixels*4*sizeof(float), out, 0, NULL, &read_out);
    if (status != CL_SUCCESS) {
        printf("clEnqueueReadBuffer() Error %d: Failed to read out buffer!\n", status);
        return 0;
    }


    // sync
    clFinish(cmdQueue);

    cl_ulong ktime_start, ktime_end;
    cl_ulong wtime_start1, wtime_end1;
    cl_ulong rtime_start, rtime_end;
    clGetEventProfilingInfo(kernel_ev, CL_PROFILING_COMMAND_START, sizeof(ktime_start), &ktime_start, NULL);
    clGetEventProfilingInfo(kernel_ev, CL_PROFILING_COMMAND_END, sizeof(ktime_end), &ktime_end, NULL);
    clGetEventProfilingInfo(read_out, CL_PROFILING_COMMAND_START, sizeof(rtime_start), &rtime_start, NULL);
    clGetEventProfilingInfo(read_out, CL_PROFILING_COMMAND_END, sizeof(rtime_end), &rtime_end, NULL);
    clGetEventProfilingInfo(write_in, CL_PROFILING_COMMAND_START, sizeof(wtime_start1), &wtime_start1, NULL);
    clGetEventProfilingInfo(write_in, CL_PROFILING_COMMAND_END, sizeof(wtime_end1), &wtime_end1, NULL);

    double total_time;
    total_time = (ktime_end-ktime_start) + (rtime_end-rtime_start) + (wtime_end1-wtime_start1);
    printf("OpenCl write execution time is: %0.3f ms \n", (wtime_end1 - wtime_start1)/1000000.0);
    printf("OpenCl read execution time is: %0.3f ms \n", (rtime_end-rtime_start)/1000000.0);
    printf("OpenCl kernel execution time is: %0.3f ms \n", (ktime_end-ktime_start)/1000000.0);
    printf("OpenCl total execution time is: %0.3f ms \n", total_time/1000000.0);

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferIn);
    clReleaseMemObject(bufferOut);
    clReleaseContext(context);
    free(platforms);
    free(devices);


    // keep the .c end

    set_output(c);
    end = clock();
    deleteGEGLclass(c);

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Overall execution time is: %0.3f ms \n", cpu_time_used * 1000);
}
