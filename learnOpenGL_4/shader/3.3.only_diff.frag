#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 normal;
struct Material{
	sampler2D texture_diffuse1;
};

layout (std140) uniform lighting{
	vec3 direction;
    vec3 ambient;
    vec3 diffuse;
};

uniform Material material;
vec3 CalcDirLight();

void main()
{    
    FragColor = vec4(CalcDirLight(), 1.f);
}

vec3 CalcDirLight(){
	// calc diff & spec
	vec3 lightDir = normalize(-direction);
	float diff = max(0.f, dot(normal, lightDir));

	vec3 ambient = ambient * vec3(texture(material.texture_diffuse1, TexCoords));
	vec3 diffuse = diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));

	return ambient + diffuse;
}