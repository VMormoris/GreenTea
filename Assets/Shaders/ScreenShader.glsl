#type vertex
#version 450 core

layout(location = 0) in vec2 _position;

out vec2 v_textCoords;

void main(void)
{
	gl_Position = vec4(_position, 0.0, 1.0);
	v_textCoords = (_position + 1.0) / 2.0;
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_color;

uniform sampler2D u_Texture;

in vec2 v_textCoords;

void main(void)
{
	vec3 color = texture2D(u_Texture, v_textCoords).rgb;
	o_color = vec4(color, 1.0);
}