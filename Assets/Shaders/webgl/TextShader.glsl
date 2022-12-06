#type vertex
//#version 330 core

attribute highp vec3 _position;
attribute highp vec4 _color;
attribute highp vec2 _textCoords;
attribute highp float _textID;
attribute highp float _objectID;
attribute highp float _screenPxRange;

uniform highp mat4 u_eyeMatrix;

varying highp vec4 v_Color;
varying highp vec2 v_textCoords;
varying highp float v_textID;
varying highp float v_ObjectID;
varying highp float v_ScreenPxRange;

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
//#version 330 core
precision highp float;

//layout(location = 0) out vec4 o_color;
//layout(location = 1) out uint o_ObjectID;

varying highp vec4 v_Color;
varying highp vec2 v_textCoords;
varying highp float v_textID;
varying highp float v_ObjectID;
varying highp float v_ScreenPxRange;

//uniform sampler2D u_Textures[32];
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform sampler2D u_Texture4;
uniform sampler2D u_Texture5;
uniform sampler2D u_Texture6;
uniform sampler2D u_Texture7;

float median(float r, float g, float b) { return max(min(r, g), min(max(r, g), b)); }

void main()
{
	
	vec3 msd = vec3(1.0, 1.0, 1.0);

	int temp = int(v_textID);
	if (temp == 0)
			msd *= texture2D(u_Texture0, v_textCoords).rgb;
		else if (temp == 1)
			msd *= texture2D(u_Texture1, v_textCoords).rgb;
		else if (temp == 2)
			msd *= texture2D(u_Texture2, v_textCoords).rgb;
		else if(temp == 3)
			msd *= texture2D(u_Texture3, v_textCoords).rgb;
		else if(temp == 4)
			msd *= texture2D(u_Texture4, v_textCoords).rgb;
		else if(temp == 5)
			msd *= texture2D(u_Texture5, v_textCoords).rgb;
		else if(temp == 6)
			msd *= texture2D(u_Texture6, v_textCoords).rgb;
		else if(temp == 7)
			msd *= texture2D(u_Texture7, v_textCoords).rgb;

	float sd = median(msd.r, msd.g, msd.b);
	float screenPxDistance = v_ScreenPxRange*(sd - 0.5);
	float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
	vec4 color = mix(vec4(0.0, 0.0, 0.0, 0.0), v_Color, opacity);
	
	if(color.a == 0.0)
		discard;

	gl_FragColor = color;
	//o_ObjectID = v_ObjectID;
}
