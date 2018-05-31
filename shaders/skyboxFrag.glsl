#version 410

in vec3 TexCoords;

uniform samplerCube skybox;
out vec4 FragColor;

void main()
{
 FragColor = texture(skybox, TexCoords);
// FragColor = vec4( TexCoords.x, TexCoords.y, 0,1);
 //FragColor = vec3(1.0f,1.0f,1.0f,1.0f);
}


