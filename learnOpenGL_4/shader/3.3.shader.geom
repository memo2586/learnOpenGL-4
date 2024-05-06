#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 18) out;

in VS_OUT{
    vec3 color;
}gs_in[];

out vec3 fColor;

const float size = 0.07f;
const float rad = 20.f;

void main() {
    // frag color
    fColor = gs_in[0].color;

    vec4 temp;
    vec4 square[4];
    float sinn = sin(radians(rad)) * size, cosn = cos(radians(rad)) * size;
    square[0] = gl_in[0].gl_Position + vec4(-sinn, sinn, 0.f, 0.f);
    square[1] = gl_in[0].gl_Position + vec4(sinn, sinn, 0.f, 0.f);
    square[2] = gl_in[0].gl_Position + vec4(sinn, -sinn, 0.f, 0.f);
    square[3] = gl_in[0].gl_Position + vec4(-sinn, -sinn, 0.f, 0.f);

    // center
    gl_Position = square[0]; EmitVertex();
    gl_Position = square[1]; EmitVertex();
    gl_Position = square[3]; EmitVertex();
    EndPrimitive();
    gl_Position = square[1]; EmitVertex();
    gl_Position = square[2]; EmitVertex();
    gl_Position = square[3]; EmitVertex();
    EndPrimitive();

    // up
    temp = gl_in[0].gl_Position + vec4(0.f, sinn + cosn, 0.f, 0.f);
    gl_Position = temp; EmitVertex();
    gl_Position = square[0]; EmitVertex();
    //gl_Position = temp; EmitVertex();
    gl_Position = square[1]; EmitVertex();
    EndPrimitive();
    // down
    temp = gl_in[0].gl_Position + vec4(0.f, -(sinn + cosn), 0.f, 0.f);
    gl_Position = temp; EmitVertex();
    gl_Position = square[3]; EmitVertex();
    //gl_Position = temp; EmitVertex();
    gl_Position = square[2]; EmitVertex();
    EndPrimitive();
    // left
    temp = gl_in[0].gl_Position + vec4(-(sinn + cosn), 0.f, 0.f, 0.f);
    gl_Position = temp; EmitVertex();
    gl_Position = square[0]; EmitVertex();
    //gl_Position = temp; EmitVertex();
    gl_Position = square[3]; EmitVertex();
    EndPrimitive();
    // right
    temp = gl_in[0].gl_Position + vec4(sinn + cosn, 0.f, 0.f, 0.f);
    gl_Position = temp; EmitVertex();
    gl_Position = square[1]; EmitVertex();
    //gl_Position = temp; EmitVertex();
    gl_Position = square[2]; EmitVertex();
    EndPrimitive();
}