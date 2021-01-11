#type vertex
#version 450 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec4 _color;
layout(location = 2) in vec2 _textCoords;
layout(location = 3) in float _textID;
layout(location = 4) in uint _objectID;

uniform mat4 u_eyeMatrix;

out vec4 v_Color;
out vec2 v_textCoords;
out float v_textID;
flat out uint v_ObjectID;

void main(void)
{
  v_Color = _color;
  v_textCoords = _textCoords;
  v_textID = _textID;
  v_ObjectID = _objectID;
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

uniform sampler2D u_Textures[32];

void main()
{
	//int index = int(v_textID);
	if(v_textID == 0.0) o_color = texture(u_Textures[0], v_textCoords) * v_Color;
	else if(v_textID == 1.0) o_color = texture(u_Textures[1], v_textCoords) * v_Color;
	else if(v_textID == 2.0) o_color = texture(u_Textures[2], v_textCoords) * v_Color;
	else if(v_textID == 3.0) o_color = texture(u_Textures[3], v_textCoords) * v_Color;
	else if(v_textID == 4.0) o_color = texture(u_Textures[4], v_textCoords) * v_Color;
	else if(v_textID == 5.0) o_color = texture(u_Textures[5], v_textCoords) * v_Color;
	else if(v_textID == 6.0) o_color = texture(u_Textures[6], v_textCoords) * v_Color;
	else if(v_textID == 7.0) o_color = texture(u_Textures[7], v_textCoords) * v_Color;
	else if(v_textID == 8.0) o_color = texture(u_Textures[8], v_textCoords) * v_Color;
	else if(v_textID == 9.0) o_color = texture(u_Textures[9], v_textCoords) * v_Color;
	else if(v_textID == 10.0) o_color = texture(u_Textures[10], v_textCoords) * v_Color;
	else if(v_textID == 11.0) o_color = texture(u_Textures[11], v_textCoords) * v_Color;
	else if(v_textID == 12.0) o_color = texture(u_Textures[12], v_textCoords) * v_Color;
	else if(v_textID == 13.0) o_color = texture(u_Textures[13], v_textCoords) * v_Color;
	else if(v_textID == 14.0) o_color = texture(u_Textures[14], v_textCoords) * v_Color;
	else if(v_textID == 15.0) o_color = texture(u_Textures[15], v_textCoords) * v_Color;
	else if(v_textID == 16.0) o_color = texture(u_Textures[16], v_textCoords) * v_Color;
	else if(v_textID == 17.0) o_color = texture(u_Textures[17], v_textCoords) * v_Color;
	else if(v_textID == 18.0) o_color = texture(u_Textures[18], v_textCoords) * v_Color;
	else if(v_textID == 19.0) o_color = texture(u_Textures[19], v_textCoords) * v_Color;
	else if(v_textID == 20.0) o_color = texture(u_Textures[20], v_textCoords) * v_Color;
	else if(v_textID == 21.0) o_color = texture(u_Textures[21], v_textCoords) * v_Color;
	else if(v_textID == 22.0) o_color = texture(u_Textures[22], v_textCoords) * v_Color;
	else if(v_textID == 23.0) o_color = texture(u_Textures[23], v_textCoords) * v_Color;
	else if(v_textID == 24.0) o_color = texture(u_Textures[24], v_textCoords) * v_Color;
	else if(v_textID == 25.0) o_color = texture(u_Textures[25], v_textCoords) * v_Color;
	else if(v_textID == 26.0) o_color = texture(u_Textures[26], v_textCoords) * v_Color;
	else if(v_textID == 27.0) o_color = texture(u_Textures[27], v_textCoords) * v_Color;
	else if(v_textID == 18.0) o_color = texture(u_Textures[28], v_textCoords) * v_Color;
	else if(v_textID == 29.0) o_color = texture(u_Textures[29], v_textCoords) * v_Color;
	else if(v_textID == 30.0) o_color = texture(u_Textures[30], v_textCoords) * v_Color;
	else if(v_textID == 31.0) o_color = texture(u_Textures[31], v_textCoords) * v_Color;

	o_ObjectID = v_ObjectID;
	//o_color = vec4(float(o_ObjectID) / 7.0, 0.0, 0.0, 1.0);
	//_color = texture(u_Textures[index], v_textCoords) * v_Color;
	//color = vec4(0.0, 0.0, 0.0, 0.0);
}
