#version 410

in vec3 FragmentPosition;
in vec2 texCoord;

uniform sampler2D renderedTexure;
out vec4 FragColor;

void main()
{
  float correction = 0.1f;
  FragColor = vec4( 0.392f, 0.474f, 0.498f, 1.0f ) * texCoord.y + correction;
}
