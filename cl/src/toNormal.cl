#include "cl/include/toNormal.h"

void kernel simple_add(global const int* A, global const int* B, global int* C)
{
  C[get_global_id(0)] = A[get_global_id(0)] + B[get_global_id(0)];
}

//-----------------------------------------------------------------------------------

void kernel toOne(global const int* A, global const int* B, global int* C)
{
  C[get_global_id(0)] = 1;
}

//-----------------------------------------------------------------------------------

//void kernel img(__read_only image2d_t src1, float * ret )
//{
//  int2 coord=(int2)(get_global_id(0), get_global_id(1));
//  if ( get_global_id(0) < 10 && get_global_id(1) == 0 )
//  {
////    ret[get_global_id(0)] =
//  }
//}
