#type vertex
#version 450 core

layout(location = 0) in vec3 _position;

uniform mat4 u_EyeModelMatrix;

void main(void)
{
	gl_Position = u_EyeModelMatrix * vec4(_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

void main(void) {/* Empty */}