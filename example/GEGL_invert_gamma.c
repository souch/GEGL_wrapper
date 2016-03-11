/*
 
 g++ -O3 GEGL_invert_gamma.c `pkg-config --libs --cflags gegl-wrapper` -o GEGL_invertgamma && ./GEGL_invertgamma car-stack.png car-stack_inverted.png

 device: AMD Phenom(tm) II X4 B93 Processor
   Execution time is: 2.422 ms


 */




#include "GEGLwrapper.h"
#include <gegl.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char* argv[]) {
  clock_t start, end;
  double cpu_time_used;
  struct GEGLclass *c = newGEGLclass(argc, argv);
  if (c == NULL)
    return 0;

  set_colorformat(c, "R'G'B'A float");
  float *in, *out;
  get_in_out(c, &in, &out);
  long samples = get_pixelcount(c);
  start = clock();
  while (samples--) {
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

}
