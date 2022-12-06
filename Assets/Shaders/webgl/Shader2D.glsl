#type vertex
//#version 330 core

attribute highp vec3 _position;
attribute highp vec4 _color;
attribute highp vec2 _textCoordsOrLocalPosition;
attribute highp float _textIDOrThickness;
attribute highp float _TilingFactorOrFade;
attribute highp float _objectID;
attribute highp float _isCircle;

uniform highp mat4 u_eyeMatrix;

varying highp vec4 v_Color;
varying highp vec2 v_textCoordsOrLocalPosition;
varying highp float v_textIDOrThickness;
varying highp float v_TilingFactorOrFade;
varying highp float v_ObjectID;
varying highp float v_isCircle;

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
//#version 330 core

precision highp float;
//layout(location = 0) out vec4 o_color;
//layout(location = 1) out uint o_ObjectID;

varying highp vec4 v_Color;
varying highp vec2 v_textCoordsOrLocalPosition;
varying highp float v_textIDOrThickness;
varying highp float v_TilingFactorOrFade;
varying highp float v_ObjectID;
varying highp float v_isCircle;

//uniform sampler2D u_Textures[32];
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform sampler2D u_Texture4;
uniform sampler2D u_Texture5;
uniform sampler2D u_Texture6;
uniform sampler2D u_Texture7;

void main()
{
	if (v_isCircle > 0.5)
	{
		float distance = 1.0 - length(v_textCoordsOrLocalPosition);
		float circle = smoothstep(0.0, v_TilingFactorOrFade, distance);
		circle *= smoothstep(v_textIDOrThickness + v_TilingFactorOrFade, v_textIDOrThickness, distance);

		if(circle == 0.0)
			discard;
		
		gl_FragColor  = v_Color;
		gl_FragColor.a *= circle;
		//o_ObjectID = v_ObjectID;
	}
	else
	{
		vec4 texColor = v_Color;

		int temp = int(v_textIDOrThickness);
		if (temp == 0)
			texColor *= texture2D(u_Texture0, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if (temp == 1)
			texColor *= texture2D(u_Texture1, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if (temp == 2)
			texColor *= texture2D(u_Texture2, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if(temp == 3)
			texColor *= texture2D(u_Texture3, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if(temp == 4)
			texColor *= texture2D(u_Texture4, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if(temp == 5)
			texColor *= texture2D(u_Texture5, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if(temp == 6)
			texColor *= texture2D(u_Texture6, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		else if(temp == 7)
			texColor *= texture2D(u_Texture7, v_textCoordsOrLocalPosition * v_TilingFactorOrFade);
		
		if (texColor.a == 0.0)
			discard;

		gl_FragColor = texColor;
		//o_ObjectID = v_ObjectID;
	}
	//o_color = vec4(float(o_ObjectID) / 7.0, 0.0, 0.0, 1.0);
	//_color = texture(u_Textures[index], v_textCoords) * v_Color;
	//color = vec4(0.0, 0.0, 0.0, 0.0);
}
