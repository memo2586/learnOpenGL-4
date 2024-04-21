#version 330 core
out vec4 fragColor;

struct Material{
	sampler2D texture_diffuse1;
	sampler2D texture_specular1;
};

#define NR_POINT_LIGHTS 4

in vec3 normal;
in vec3 fragPos;
in vec2 texCoord;

float near = 0.1; 
float far  = 100.0; 

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));    
}

void main()
{   
    // 线性
    // fragColor = vec4(vec3(gl_FragCoord.z), 1.f);

    // 非线性
    float depth = LinearizeDepth(gl_FragCoord.z) / far;
    fragColor = vec4(vec3(depth), 1.0);
}
