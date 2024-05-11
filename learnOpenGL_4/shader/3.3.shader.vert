#version 330 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec3 vertColor;
layout (location = 2) in vec2 offset;

out vec3 color;

void main(){
	gl_Position = vec4(vertPos  * (gl_InstanceID / 100.f) + offset, 0.f, 1.f);
	color = vertColor;
}