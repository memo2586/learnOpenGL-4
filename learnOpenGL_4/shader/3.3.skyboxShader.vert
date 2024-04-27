#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    // gl_Position = projection * view * vec4(aPos, 1.0);

    // 优化：让天空盒的深度恒为1，永远在其他物体后，深度测试时会将被遮挡的片段丢弃
    vec4 pos = projection * view * vec4(aPos, 1.0);
    // NDC 指标下的 z 分量 = w / w = 1
    gl_Position = pos.xyww;
}