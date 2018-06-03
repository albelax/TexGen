#version 400

layout(vertices = 3) out;

in vec3 vPosition[];
in vec3 vNormal[];
in vec2 vUV[];

out vec3 tcPosition[];
out vec3 tcNormal[];
out vec2 tcUV[];
uniform float TessLevelInner = 64;
uniform float TessLevelOuter = 64;


void main()
{
  tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
  tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
  tcUV[gl_InvocationID] = vUV[gl_InvocationID];
  if (gl_InvocationID == 0)
  {
    gl_TessLevelInner[0] = TessLevelInner;
    gl_TessLevelOuter[0] = TessLevelOuter;
    gl_TessLevelOuter[1] = TessLevelOuter;
    gl_TessLevelOuter[2] = TessLevelOuter;
  }
}
