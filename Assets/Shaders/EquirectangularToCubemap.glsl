#type vertex
#version 450 core

layout(location = 0) in vec3 _position;

uniform mat4 u_EyeMatrix;

out vec3 v_WorldPos;


void main(void)
{
	v_WorldPos = _position;
	gl_Position = u_EyeMatrix * vec4(_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

const vec2 invAtan = vec2(0.1591, 0.3183);

uniform sampler2D EquirectangularMap;

in vec3 v_WorldPos;

vec2 SampleSphericalMap(const vec3 v) { return vec2(atan(v.z, v.x), asin(v.y)) * invAtan + 0.5; }

void main()
{
	const vec2 uv = SampleSphericalMap(normalize(v_WorldPos));
	const vec3 color = texture(EquirectangularMap, uv).rgb;
	o_Color = vec4(color, 1.0);
}
