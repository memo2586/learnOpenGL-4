#version 330 core
out vec4 fragColor;

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

void main(){
	fragColor = vec4(.8f, .6f, .4f, 1.f);
}