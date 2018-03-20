#version 410

// This is passed on from the vertex shader
in vec3 FragmentPosition;
in vec3 FragmentNormal;
in vec2 texCoord;

uniform sampler2D ColourTexture;
uniform sampler2D NormalTexture;


layout ( location = 0 ) out vec4 FragColor;

mat4 rotationMatrix( vec3 axis, float angle )
{
    //axis = normalize(axis);
    float s = sin( angle );
    float c = cos( angle );
    float oc = 1.0 - c;
    return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0 );
}

vec3 rotateVector( vec3 src, vec3 tgt, vec3 vec )
{
  float angle = acos( dot( src, tgt ) );

  if ( angle == 0 )
  {
    return vec;
  }
  vec3 axis = normalize( cross( src, tgt ) );
  mat4 R = rotationMatrix( axis, angle );
  vec4 norm = R * vec4( vec, 1.0f );
  return norm.xyz / norm.w;
}


float texture_lum(sampler2D _texture, vec2 _uv)
{
  vec3 rgb = texture( _texture, _uv ).rgb;
  return 0.2126 * rgb.r + 0.7152 * rgb.g + 0.0722 * rgb.b;
}

vec3 toNormal()
{
  float r = 0.00078125f; // 1 / width of the texture

  float x0 = texture_lum( ColourTexture, vec2( texCoord.x + r, texCoord.y ) );
  float x1 = texture_lum( ColourTexture, vec2( texCoord.x - r, texCoord.y ) );
  float y0 = texture_lum( ColourTexture, vec2( texCoord.x, texCoord.y + r ) );
  float y1 = texture_lum( ColourTexture, vec2( texCoord.x, texCoord.y - r ) );

  vec3 n = normalize( vec3( x1 - x0, y1 - y0, 1.0f ) );

  return n * 0.5 + 0.5;
}


void main()
{
  vec3 n = normalize( FragmentNormal ); // normal

  vec3 v = normalize( -vec3( FragmentPosition ) ); // eye

  vec3 tgt = normalize( toNormal() * 2.0 - 1.0 );

  vec3 src = vec3( 0.0, 0.0, 1.0 );
  n = rotateVector( src, tgt, n);

  FragColor = vec4( n, 1);
}
