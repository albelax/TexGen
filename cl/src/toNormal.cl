#include "cl/include/toNormal.h"

void kernel simple_add(global const int* A, global const int* B, global int* C)
{
  int2 coord=(int2)(get_global_id(0), get_global_id(1));
  C[get_global_id(0)] = A[get_global_id(0)] + B[get_global_id(0)];
//  printf("idx: %d, idy: %d\n", coord.x, coord.y);
}

//-----------------------------------------------------------------------------------

void kernel toOne(global const int* A, global const int* B, global int* C)
{
  C[get_global_id(0)] = 1;
}

//-----------------------------------------------------------------------------------

void kernel img(__read_only image2d_t src1, global float * ret )
{
  int2 coord=(int2)(get_global_id(0), get_global_id(1));
  //  if ( get_global_id(0) < 10 && get_global_id(1) == 0 )
  //  {
  //    ret[get_global_id(0)] =
  //  }
}

//-----------------------------------------------------------------------------------

void kernel calculateMap( global float * _r, global float * _g, global float * _b )
{
  int2 uv = (int2)(get_global_id(0), get_global_id(1));

  int _width = 2196;

  float r = 1.0f;// / _width;
  //  float3   rgb = (float3)( _r[ uv.y * _width + uv.x ], _g[ uv.y * _width + uv.x ], _b[ uv.y * _width + uv.x ]);

  float3 rgb = (float3)( _r[ uv.y * _width + (int)(uv.x + r) ], _g[ uv.y * _width + (int)(uv.x + r) ], _b[ uv.y * _width + (int)(uv.x + r) ]);
  float x0 = 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;
  rgb = (float3)( _r[ uv.y * _width + (int)(uv.x - r) ], _g[ uv.y * _width + (int)(uv.x - r) ], _b[ uv.y * _width + (int)(uv.x - r) ]);
  float x1 = 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;

  rgb = (float3)( _r[ (int)(uv.y + r ) * _width + uv.x ], _g[ (int)(uv.y + r ) * _width + uv.x ], _b[ (int)(uv.y + r ) * _width + uv.x ]);
  float y0 = 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;
  rgb = (float3)( _r[ (int)(uv.y - r ) * _width + uv.x ], _g[ (int)(uv.y - r ) * _width + uv.x ], _b[ (int)(uv.y - r ) * _width + uv.x ]);
  float y1 = 0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z;

  float3 n = normalize( (float3)( x1 - x0, y1 - y0, 1.0f ));

  _r[ uv.y * _width + uv.x ] = ( n.x * 0.5 ) + 0.5;
  _g[ uv.y * _width + uv.x ] = ( n.y * 0.5 ) + 0.5;
  _b[ uv.y * _width + uv.x ] = ( n.z * 0.5 ) + 0.5;

}
