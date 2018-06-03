#version 400

layout (location = 0) in vec3 VertexPosition;
/// @brief the normal passed in
layout (location = 2) in vec3 VertexNormal;
/// @brief the in uv
layout (location = 1) in vec2 TexCoord;

uniform mat4 MVP;

out vec3 vPosition;
out vec3 vNormal;
out vec2 vUV;

void main()
{
  vPosition = VertexPosition;
  vNormal = VertexNormal;
  vUV = TexCoord;
}
