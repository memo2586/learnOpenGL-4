#version 330 core
out vec4 FragColor;

in vec3 normal;
in vec3 fragPos;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};

uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{             
//    vec3 I = normalize(fragPos - viewPos);
//    vec3 R = reflect(I, normalize(normal)); 
//
//    FragColor = vec4(texture(skybox, R).rgb, 1.f);

	float ratio = 1.00 / 1.52;
    vec3 I = normalize(fragPos - viewPos);
    vec3 R = refract(I, normalize(normal), ratio);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}
