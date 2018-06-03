#version 400

uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 N;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tePosition[3];
in vec3 teNormal[3];
in vec2 teUV[3];

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;

uniform sampler2D ColourTexture;

void main()
{
  float dispScale = 0.2;
  float displacement = texture(ColourTexture, teUV[0]).r * dispScale;
  Normal = normalize(teNormal[0]);
  vec4 newPos =  vec4(tePosition[0] +Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[0];
  gl_Position = MVP * newPos; EmitVertex();


  displacement = texture(ColourTexture, teUV[1]).r * dispScale;
  Normal = normalize(teNormal[1]);
  newPos = vec4(tePosition[1] + Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[1];
  gl_Position = MVP * newPos; EmitVertex();


  displacement = texture(ColourTexture, teUV[2]).r * dispScale;
  Normal = normalize(teNormal[2]);
  newPos =  vec4(tePosition[2] + Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[2];
  gl_Position = MVP * newPos; EmitVertex();

  EndPrimitive();
}

