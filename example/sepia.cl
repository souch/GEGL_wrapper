// v1
__kernel void cl_sepia(__global const float4 *in,
                       const float scale,
                       __global       float4 *out)
{ 
  int gid = get_global_id(0);
  
  //float scale = 0.9; //  o->scale

  __local float m[9];
  m[0] = 0.393 + 0.607 * (1.0 - scale);
  m[1] = 0.769 - 0.769 * (1.0 - scale);
  m[2] = 0.189 - 0.189 * (1.0 - scale);

  m[3] = 0.349 - 0.349 * (1.0 - scale);
  m[4] = 0.686 + 0.314 * (1.0 - scale);
  m[5] = 0.168 - 0.168 * (1.0 - scale);

  m[6] = 0.272 - 0.272 * (1.0 - scale);
  m[7] = 0.534 - 0.534 * (1.0 - scale);
  m[8] = 0.131 + 0.869 * (1.0 - scale);

  float4 in_v = in[gid];
  float4 out_v;
  out_v.x = m[0] * in_v.x + m[1] * in_v.y + m[2] * in_v.z;
  out_v.y = m[3] * in_v.x + m[4] * in_v.y + m[5] * in_v.z;
  out_v.z = m[6] * in_v.x + m[7] * in_v.y + m[8] * in_v.z;
  out_v.w = in_v.w;
  out[gid] = out_v;
}

