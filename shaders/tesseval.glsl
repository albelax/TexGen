#version 400

layout(triangles, equal_spacing, cw) in;

in vec3 tcPosition[];
in vec3 tcNormal[];
in vec2 tcUV[];

out vec3 tePosition;
out vec3 teNormal;
out vec2 teUV;
uniform mat4 MVP;

#define ID gl_TessCoord

void main()
{
  // ID = barycentric coordinate
  tePosition = (ID.x * tcPosition[0] + ID.y * tcPosition[1] + ID.z * tcPosition[2]);
  teNormal = (ID.x * tcNormal[0] + ID.y * tcNormal[1] + ID.z * tcNormal[2]);
  teUV = (ID.x * tcUV[0] + ID.y * tcUV[1] + ID.z * tcUV[2]);
  gl_Position =  MVP * vec4(tePosition, 1.0);
}
