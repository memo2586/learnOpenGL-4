#version 330 core
out vec4 fragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
	float shininess;
};

in vec2 TexCoord;

uniform Material material;

void main(){	
	fragColor = vec4(vec3(texture(material.texture_diffuse1, TexCoord)), 1.f);
}
