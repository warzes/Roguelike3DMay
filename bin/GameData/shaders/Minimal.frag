#version 460 core

in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

layout(binding = 0) uniform sampler2D diffuseTex;

vec3 lightPos = vec3(4.0, 5.0, -3.0);
vec3 lightColor = vec3(1.0, 1.0, 1.0);

layout(location = 0) out vec4 FragColor;

void main()
{
	float lightAngle = max(dot(normalize(fs_in.Normal), normalize(lightPos)), 0.0);
	vec4 color = texture(diffuseTex, fs_in.TexCoords);

	FragColor = vec4(color.rgb, color.a) * vec4((0.3 + 0.7 * lightAngle) * lightColor, 1.0);
}