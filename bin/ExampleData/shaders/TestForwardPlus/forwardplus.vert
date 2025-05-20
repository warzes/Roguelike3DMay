#version 460 core

layout (location = 0) in vec3 vert_pos;
layout (location = 1) in vec3 vert_normal;
layout (location = 2) in vec2 vert_uv;

out VERTEX_OUT {
  vec2 frag_uv;
  vec3 fragNormal;
  vec3 fragPos;
} vertex_out;

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
	gl_Position = projection * view * model * vec4(vert_pos, 1.0);
	mat4 invTransModel = transpose(inverse(model)); // TODO: можно передать с CPU

	vertex_out.frag_uv = vert_uv;
	vertex_out.fragNormal = normalize((invTransModel * vec4(vert_normal, 0.0)).xyz);
	vertex_out.fragPos = vec3(model * vec4(vert_pos, 1.0));
}