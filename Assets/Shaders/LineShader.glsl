#type vertex
#version 450 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec4 _color;

uniform mat4 u_eyeMatrix;

out vec4 v_Color;

void main()
{
	v_Color = _color;
	gl_Position = u_eyeMatrix * vec4(_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_color;

in vec4 v_Color;

void main()
{
	o_color = v_Color;
}