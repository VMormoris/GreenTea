#type vertex
#version 450 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec4 _color;
layout(location = 2) in vec2 _textCoords;
layout(location = 3) in float _textID;
layout(location = 4) in uint _objectID;
layout(location = 5) in float _screenPxRange;

uniform mat4 u_eyeMatrix;

out vec4 v_Color;
out vec2 v_textCoords;
out float v_textID;
flat out uint v_ObjectID;
out float v_ScreenPxRange;

void main(void)
{
	v_Color = _color;
	v_textCoords = _textCoords;
	v_textID = _textID;
	v_ObjectID = _objectID;
	v_ScreenPxRange = _screenPxRange;
	gl_Position = u_eyeMatrix * vec4(_position, 1.0);
}


#type fragment
#version 450 core

layout(location = 0) out vec4 o_color;
layout(location = 1) out uint o_ObjectID;

in vec4 v_Color;
in vec2 v_textCoords;
in float v_textID;
flat in uint v_ObjectID;
in float v_ScreenPxRange;

uniform sampler2D u_Textures[32];

float median(float r, float g, float b) { return max(min(r, g), min(max(r, g), b)); }

void main()
{
	
	vec3 msd = vec3(1.0, 1.0, 1.0);
	switch(int(v_textID))
	{
		case  0: msd *= texture(u_Textures[ 0], v_textCoords).rgb; break;
		case  1: msd *= texture(u_Textures[ 1], v_textCoords).rgb; break;
		case  2: msd *= texture(u_Textures[ 2], v_textCoords).rgb; break;
		case  3: msd *= texture(u_Textures[ 3], v_textCoords).rgb; break;
		case  4: msd *= texture(u_Textures[ 4], v_textCoords).rgb; break;
		case  5: msd *= texture(u_Textures[ 5], v_textCoords).rgb; break;
		case  6: msd *= texture(u_Textures[ 6], v_textCoords).rgb; break;
		case  7: msd *= texture(u_Textures[ 7], v_textCoords).rgb; break;
		case  8: msd *= texture(u_Textures[ 8], v_textCoords).rgb; break;
		case  9: msd *= texture(u_Textures[ 9], v_textCoords).rgb; break;
		case 10: msd *= texture(u_Textures[10], v_textCoords).rgb; break;
		case 11: msd *= texture(u_Textures[11], v_textCoords).rgb; break;
		case 12: msd *= texture(u_Textures[12], v_textCoords).rgb; break;
		case 13: msd *= texture(u_Textures[13], v_textCoords).rgb; break;
		case 14: msd *= texture(u_Textures[14], v_textCoords).rgb; break;
		case 15: msd *= texture(u_Textures[15], v_textCoords).rgb; break;
		case 16: msd *= texture(u_Textures[16], v_textCoords).rgb; break;
		case 17: msd *= texture(u_Textures[17], v_textCoords).rgb; break;
		case 18: msd *= texture(u_Textures[18], v_textCoords).rgb; break;
		case 19: msd *= texture(u_Textures[19], v_textCoords).rgb; break;
		case 20: msd *= texture(u_Textures[20], v_textCoords).rgb; break;
		case 21: msd *= texture(u_Textures[21], v_textCoords).rgb; break;
		case 22: msd *= texture(u_Textures[22], v_textCoords).rgb; break;
		case 23: msd *= texture(u_Textures[23], v_textCoords).rgb; break;
		case 24: msd *= texture(u_Textures[24], v_textCoords).rgb; break;
		case 25: msd *= texture(u_Textures[25], v_textCoords).rgb; break;
		case 26: msd *= texture(u_Textures[26], v_textCoords).rgb; break;
		case 27: msd *= texture(u_Textures[27], v_textCoords).rgb; break;
		case 28: msd *= texture(u_Textures[28], v_textCoords).rgb; break;
		case 29: msd *= texture(u_Textures[29], v_textCoords).rgb; break;
		case 30: msd *= texture(u_Textures[30], v_textCoords).rgb; break;
		case 31: msd *= texture(u_Textures[31], v_textCoords).rgb; break;
	}

	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = v_ScreenPxRange*(sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	vec4 color = mix(vec4(0.0, 0.0, 0.0, 0.0), v_Color, opacity);
	
	if(color.a == 0.0)
		discard;

	o_color = color;
	o_ObjectID = v_ObjectID;
}
