#type vertex
#version 330 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec4 _color;
layout(location = 2) in vec2 _textCoordsOrLocalPosition;
layout(location = 3) in float _textIDOrThickness;
layout(location = 4) in float _TilingFactorOrFade;
layout(location = 5) in uint _objectID;
layout(location = 6) in float _isCircle;

uniform mat4 u_eyeMatrix;

out vec4 v_Color;
out vec2 v_textCoordsOrLocalPosition;
out float v_textIDOrThickness;
out float v_TilingFactorOrFade;
flat out uint v_ObjectID;
flat out float v_isCircle;

void main(void)
{
	v_Color = _color;
	v_textCoordsOrLocalPosition = _textCoordsOrLocalPosition;
	v_textIDOrThickness = _textIDOrThickness;
	v_TilingFactorOrFade = _TilingFactorOrFade;
	v_ObjectID = _objectID;
	v_isCircle = _isCircle;
	gl_Position = u_eyeMatrix * vec4(_position, 1.0);
}


#type fragment
#version 330 core

layout(location = 0) out vec4 o_color;
layout(location = 1) out uint o_ObjectID;

in vec4 v_Color;
in vec2 v_textCoordsOrLocalPosition;
in float v_textIDOrThickness;
in float v_TilingFactorOrFade;
flat in uint v_ObjectID;
flat in float v_isCircle;

uniform sampler2D u_Textures[32];

void main()
{
	if (v_isCircle > 0.5)
	{
		float distance = 1.0 - length(v_textCoordsOrLocalPosition);
		float circle = smoothstep(0.0, v_TilingFactorOrFade, distance);
		circle *= smoothstep(v_textIDOrThickness + v_TilingFactorOrFade, v_textIDOrThickness, distance);

		if(circle == 0.0)
			discard;
		
		o_color = v_Color;
		o_color.a *= circle;
		o_ObjectID = v_ObjectID;
	}
	else
	{
		vec4 texColor = v_Color;

		switch(int(v_textIDOrThickness))
		{
			case  0: texColor *= texture(u_Textures[ 0], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  1: texColor *= texture(u_Textures[ 1], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  2: texColor *= texture(u_Textures[ 2], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  3: texColor *= texture(u_Textures[ 3], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  4: texColor *= texture(u_Textures[ 4], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  5: texColor *= texture(u_Textures[ 5], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  6: texColor *= texture(u_Textures[ 6], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  7: texColor *= texture(u_Textures[ 7], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  8: texColor *= texture(u_Textures[ 8], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case  9: texColor *= texture(u_Textures[ 9], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 10: texColor *= texture(u_Textures[10], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 11: texColor *= texture(u_Textures[11], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 12: texColor *= texture(u_Textures[12], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 13: texColor *= texture(u_Textures[13], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 14: texColor *= texture(u_Textures[14], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 15: texColor *= texture(u_Textures[15], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 16: texColor *= texture(u_Textures[16], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 17: texColor *= texture(u_Textures[17], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 18: texColor *= texture(u_Textures[18], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 19: texColor *= texture(u_Textures[19], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 20: texColor *= texture(u_Textures[20], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 21: texColor *= texture(u_Textures[21], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 22: texColor *= texture(u_Textures[22], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 23: texColor *= texture(u_Textures[23], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 24: texColor *= texture(u_Textures[24], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 25: texColor *= texture(u_Textures[25], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 26: texColor *= texture(u_Textures[26], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 27: texColor *= texture(u_Textures[27], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 28: texColor *= texture(u_Textures[28], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 29: texColor *= texture(u_Textures[29], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 30: texColor *= texture(u_Textures[30], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
			case 31: texColor *= texture(u_Textures[31], v_textCoordsOrLocalPosition * v_TilingFactorOrFade); break;
		}

		if (texColor.a == 0.0)
			discard;

		o_color = texColor;
		o_ObjectID = v_ObjectID;
	}
	//o_color = vec4(float(o_ObjectID) / 7.0, 0.0, 0.0, 1.0);
	//_color = texture(u_Textures[index], v_textCoords) * v_Color;
	//color = vec4(0.0, 0.0, 0.0, 0.0);
}
