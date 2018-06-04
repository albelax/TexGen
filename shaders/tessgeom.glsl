#version 400

uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 N;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 tePosition[3];
in vec3 teNormal[3];
in vec2 teUV[3];

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform sampler2D ColourTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DisplacementTexture;
uniform int tiling;


void main()
{
	vec2 uv = teUV[0] * tiling;

  float dispScale = 0.2;
	float displacement = texture(DisplacementTexture, uv ).r * dispScale;

	vec3 tgt = normalize( texture( NormalTexture, uv ).rgb * 2.0 - 1.0 );
	vec3 src = vec3( 0.0, 0.0, 1.0 );

	Normal = normalize( teNormal[0] );
  vec4 newPos =  vec4(tePosition[0] +Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[0];
  gl_Position = MVP * newPos; EmitVertex();

	uv = teUV[1] * tiling;
	displacement = texture(DisplacementTexture, uv ).r * dispScale;
	Normal = normalize(teNormal[1]);
  newPos = vec4(tePosition[1] + Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[1];
  gl_Position = MVP * newPos; EmitVertex();

	uv = teUV[2] * tiling;
	displacement = texture(DisplacementTexture, uv).r * dispScale;
	Normal = normalize(teNormal[2]);
  newPos =  vec4(tePosition[2] + Normal * displacement, 1.0);
  WorldPos = vec3(MV * newPos);
  TexCoords = teUV[2];
  gl_Position = MVP * newPos; EmitVertex();

  EndPrimitive();
}

