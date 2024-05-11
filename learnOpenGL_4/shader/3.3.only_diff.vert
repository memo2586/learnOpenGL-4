#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 normal;

layout (std140) uniform mvp{
    mat4 model;
    mat4 view;
    mat4 projection;
};

void main()
{
    TexCoords = aTexCoords;
    mat4 nrmMat = inverse(transpose(model));
    normal = normalize(nrmMat * vec4(aNormal, 1.f)).xyz;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}