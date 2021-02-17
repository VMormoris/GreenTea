#type vertex
#version 450 core

layout(location = 0) in vec3 _position;

uniform mat4 u_EyeModelMatrix;

out vec3 v_TextCoords;

void main()
{
	v_TextCoords = _position;
	vec4 pos = u_EyeModelMatrix * vec4(_position, 1.0);
	gl_Position = pos.xyww;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out uint o_ObjectID;

uniform samplerCube Skybox;

in vec3 v_TextCoords;

void main()
{
	o_Color = texture(Skybox, v_TextCoords);
}