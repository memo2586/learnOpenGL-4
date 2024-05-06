#version 330 core
layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec3 vertColor;

out VS_OUT{
	vec3 color;
}vs_out;

void main(){
	vs_out.color = vertColor;
	gl_Position = vec4(vertPos, 0.f, 1.f);
}