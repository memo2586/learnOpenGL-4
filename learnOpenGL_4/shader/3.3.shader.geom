#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec2[] TexCoord;
out vec2 texCoord;

uniform float time;

vec4 explode(vec4 position, vec3 normal);

void main(){
	vec3 a = vec3(gl_in[0].gl_Position - gl_in[1].gl_Position);
	vec3 b = vec3(gl_in[2].gl_Position - gl_in[1].gl_Position);
	vec3 normal = normalize(cross(a, b));

	for(int i = 0; i < 3; i++){
		gl_Position = explode(gl_in[i].gl_Position, normal);
		texCoord = TexCoord[i];
		EmitVertex();
	}
	EndPrimitive();
}

vec4 explode(vec4 position, vec3 normal){
	float level = 1.f;
	vec3 direction = normal * ((sin(time) + 1.f) / 2.f) * level;
	return position + vec4(direction, 0.f);
}