#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
struct Material{
	sampler2D texture_diffuse1;
};

uniform Material material;

// ע�ⶨ���˳��Ӧ�ú�buffer�����ݵ�˳����ͬ
layout (std140) uniform block1{
    vec3 v2;
    int i;
    int j;
};

void main()
{   
    FragColor = vec4(v2, 1.f);
    if(i==12) FragColor *= .2f;
}