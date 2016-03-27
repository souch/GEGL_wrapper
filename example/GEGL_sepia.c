/*
 
 device: AMD Phenom(tm) II X4 B93 Processor

 g++ -O3 GEGL_sepia.c `pkg-config --libs --cflags gegl-wrapper` -o GEGL_sepia && ./GEGL_sepia car-stack.png car-stack_sepia.png

 Execution time is: 2.491 ms

 g++ -O3  GEGL_sepia.c `pkg-config --libs --cflags gegl-wrapper` -o GEGL_sepia && ./GEGL_sepia tofukiwi.jpg tofukiwi_sepia.jpg

Execution time is: 211.781 ms

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
//  typedef float v4sf __attribute__ ((vector_size (16)));
//  v4sf *vin, *vout;
//  vin = (*v4sf) in;
//  vout = (*v4sf) out;

  printf("f:%d\n", sizeof(float));
     
  gfloat m[25] = { 1.0, 0.0, 0.0, 0.0, 0.0,
                   0.0, 1.0, 0.0, 0.0, 0.0,
                   0.0, 0.0, 1.0, 0.0, 0.0,
                   0.0, 0.0, 0.0, 1.0, 0.0,
                   0.0, 0.0, 0.0, 0.0, 1.0 };
  gfloat scale = 0.9; //  o->scale
  glong i;

  m[0] = 0.393 + 0.607 * (1.0 - scale);
  m[1] = 0.769 - 0.769 * (1.0 - scale);
  m[2] = 0.189 - 0.189 * (1.0 - scale);

  m[5] = 0.349 - 0.349 * (1.0 - scale);
  m[6] = 0.686 + 0.314 * (1.0 - scale);
  m[7] = 0.168 - 0.168 * (1.0 - scale);

  m[10] = 0.272 - 0.272 * (1.0 - scale);
  m[11] = 0.534 - 0.534 * (1.0 - scale);
  m[12] = 0.131 + 0.869 * (1.0 - scale);

  start = clock();
  for (i = 0; i < samples; i++)
    {
      out[0] = m[0] * in[0] + m[1] * in[1] + m[2] * in[2];
      out[1] = m[5] * in[0] + m[6] * in[1] + m[7] * in[2];
      out[2] = m[10] * in[0] + m[11] * in[1] + m[12] * in[2];
      out[3] = in[3];

      in += 4;
      out += 4;
    }
  end = clock();

  set_output(c);
  deleteGEGLclass(c);
  
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Execution time is: %0.3f ms \n", cpu_time_used * 1000);

}
