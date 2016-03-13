/*

device: AMD Phenom(tm) II X4 B93 Processor

g++ -O3 -fopenmp GEGL_invertgamma_openmp.c `pkg-config --libs --cflags gegl-wrapper` -o GEGL_invertgamma_openmp && ./GEGL_invertgamma_openmp car-stack.png car-stack_inverted.png && /opt/ocl/GEGL-OpenCL/tools/gegl-imgcmp car-stack_inverted.png car-stack_inverted_ori.png

Execution time is: 7.387 ms

g++ -O3 -fopenmp GEGL_invertgamma_openmp.c `pkg-config --libs --cflags gegl-wrapper` -o GEGL_invertgamma_openmp && ./GEGL_invertgamma_openmp tofukiwi.jpg tofukiwi_inverted.jpg && /opt/ocl/GEGL-OpenCL/tools/gegl-imgcmp tofukiwi_inverted.jpg tofukiwi_inverted_ori.jpg

Execution time is: 496.732 ms

=> much worse than sequntial one (2.5 and 200ms), memory bound?
*/




#include "GEGLwrapper.h"
#include <gegl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

int main(int argc, char* argv[]) {
    clock_t start, end;
    double cpu_time_used;
    struct GEGLclass *c = newGEGLclass(argc, argv);
    if (c == NULL)
        return 0;

    set_colorformat(c, "R'G'B'A float");
    float *in, *out;
    get_in_out(c, &in, &out);
    const long samples = get_pixelcount(c);
    printf("nb samples: %d\n", samples);

    //printf("in:%p, out:%p\n", in, out);
    
    start = clock();
    int th_size;
    int th_id, nthreads;
    float *th_in, *th_out;
    #pragma omp parallel private(nthreads, th_id, th_in, th_out, th_size) 
    {
        nthreads = omp_get_num_threads();
        th_id = omp_get_thread_num();
        th_size = samples / nthreads;
        th_in = in + (th_size * 4 * th_id);
        th_out = out + (th_size * 4 * th_id);
        //printf("thread %d do %d samples, in:%p, out:%p\n", th_id, th_size, th_in, th_out);
        while (th_size--) {
            th_out[0] = 1.0 - th_in[0];
            th_out[1] = 1.0 - th_in[1];
            th_out[2] = 1.0 - th_in[2];
            th_out[3] = th_in[3];
            th_in += 4;
            th_out+= 4;
        }
    }
    end = clock();


    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Execution time is: %0.3f ms \n", cpu_time_used * 1000);

    set_output(c);
    deleteGEGLclass(c);

}


