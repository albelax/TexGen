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

void kernel calculateMap( global float * _r, global float * _g, global float * _b,
                          global float * o_r, global float * o_g, global float * o_b,
                          const int _width, const int _depth, const int _invert )
{
  int2 uv = (int2)(get_global_id(0), get_global_id(1));

  float r = 1.0 / _width;

  float depth = _depth;

  float3 rgb = (float3)( _r[ uv.y * _width + (int)(uv.x + r) ], _g[ uv.y * _width + (int)(uv.x + r) ], _b[ uv.y * _width + (int)(uv.x + r) ]);
  float x0 = (0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z) * depth;

  rgb = (float3)( _r[ uv.y * _width + (int)(uv.x - r) ], _g[ uv.y * _width + (int)(uv.x - r) ], _b[ uv.y * _width + (int)(uv.x - r) ]);
  float x1 = (0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z) * depth;

  rgb = (float3)( _r[ (int)(uv.y + r ) * _width + uv.x ], _g[ (int)(uv.y + r ) * _width + uv.x ], _b[ (int)(uv.y + r ) * _width + uv.x ]);
  float y0 = (0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z) * depth;

  rgb = (float3)( _r[ (int)(uv.y - r ) * _width + uv.x ], _g[ (int)(uv.y - r ) * _width + uv.x ], _b[ (int)(uv.y - r ) * _width + uv.x ]);
  float y1 = (0.2126 * rgb.x + 0.7152 * rgb.y + 0.0722 * rgb.z) * depth;

  float3 n;

  if ( _invert )
     n = normalize( (float3)( (1-x1) - (1-x0), (1-y1) - (1-y0), 1.0f ));
  else
     n = normalize( (float3)( x1 - x0, y1 - y0, 1.0f ));

  o_r[ uv.y * _width + uv.x ] = ( n.x * 0.5 ) + 0.5;
  o_g[ uv.y * _width + uv.x ] = ( n.y * 0.5 ) + 0.5;
  o_b[ uv.y * _width + uv.x ] = ( n.z * 0.5 ) + 0.5;
}
