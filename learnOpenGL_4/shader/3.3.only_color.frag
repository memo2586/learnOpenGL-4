#version 330 core
out vec4 fragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};

void main(){
	fragColor = vec4(0.04, 0.28, 0.26, 1.0);;
}