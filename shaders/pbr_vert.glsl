#version 420 // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 VertexPosition;
/// @brief the normal passed in
layout (location = 2) in vec3 VertexNormal;
/// @brief the in uv
layout (location = 1) in vec2 TexCoord;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 MVP;
uniform mat4 N;
uniform mat4 MV;

void main()
{
  gl_Position = MVP * vec4(VertexPosition, 1.0);
  WorldPos = vec3(MV * vec4(VertexPosition, 1.0));
  Normal = vec3(N * vec4(VertexNormal, 1.0));
  TexCoords = TexCoord;
}
