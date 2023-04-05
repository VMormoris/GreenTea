#type vertex
#version 450 core

layout(location = 0) in vec3 _position;

uniform mat4 u_EyeMatrix;

out vec3 v_Pos;


void main(void)
{
	v_Pos = _position;
	const vec4 pos = u_EyeMatrix * vec4(_position, 1.0);
	gl_Position = pos.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

uniform samplerCube EnvironmentMap;

in vec3 v_Pos;

void main()
{
	vec3 color = texture(EnvironmentMap, v_Pos).rgb;
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0 / 2.2));
	o_Color = vec4(color, 1.0);
}
