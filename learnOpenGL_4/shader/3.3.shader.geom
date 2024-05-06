#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vec2[] TexCoord;
in vec3[] normal;
out vec2 texCoord;

const float dis = .05f;

void main(){
	for(int i = 0; i < 3; i++){
		texCoord = TexCoord[i];
		gl_Position = gl_in[i].gl_Position;
		EmitVertex();
		gl_Position = gl_in[i].gl_Position + vec4(normalize(normal[i]), 0.f) * dis;
		EmitVertex();
		EndPrimitive();
	}
}