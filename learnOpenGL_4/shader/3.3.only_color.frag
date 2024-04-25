#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D frame_texture;

void main()
{ 
    FragColor = texture(frame_texture, TexCoords);
}