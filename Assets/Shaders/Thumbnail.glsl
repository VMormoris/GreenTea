#type vertex
#version 450 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec3 _normal;
layout(location = 2) in vec2 _textCoords;
layout(location = 3) in vec3 _tangent;
layout(location = 4) in vec3 _bitangent;

uniform mat4 u_EyeMatrix;
uniform mat4 u_ModelMatrix;
uniform mat4 u_NormalMatrix;

out vec2 v_TextCoords;
out vec3 v_Position_WCS;
out mat3 v_TBN;

void main(void)
{
	v_TBN = mat3(
		normalize(vec3(u_NormalMatrix * vec4(_tangent, 0.0))),
		normalize(vec3(u_NormalMatrix * vec4(_bitangent, 0.0))),
		normalize(vec3(u_NormalMatrix * vec4(_normal, 0.0))));

	v_TextCoords = _textCoords;
	v_Position_WCS = vec3(u_ModelMatrix * vec4(_position, 1.0));
	gl_Position = u_EyeMatrix * vec4(v_Position_WCS, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

const float _PI_ = 3.14159;
const vec3 LightDir = vec3(0.0, 0.0, -1.0);
const vec3 LightPos = vec3(0.0, 0.0, 100.0);
const vec3 LightColor = vec3(1.0, 1.0, 1.0);

uniform vec4 u_Diffuse;
uniform vec4 u_EmitColor;
uniform vec4 u_AmbientColor;
uniform float u_Metallicness;
uniform float u_Shininess;

uniform bool u_HasAlbedo;
uniform bool u_HasNormal;
uniform bool u_HasMetallic;
uniform bool u_HasEmissive;
uniform bool u_HasOcclusion;
uniform bool u_HasOpacity;

uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;

uniform vec2 u_ViewportSize;

uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D OclussionTexture;
uniform sampler2D OpacityTexture;
uniform sampler2D EmissiveTexture;

in vec2 v_TextCoords;
in vec3 v_Position_WCS;
in vec3 v_Normal;
in mat3 v_TBN;

vec3 fresnel
(
	const in vec3 diffColor,
	const in float VdotH,
	const in float metallic,
	const in float shininess
)
{
   const vec3 f0 = mix(diffColor * 0.08, vec3(0.04), metallic);
   return f0 + (vec3(1.0) - f0) * pow(1.0 - VdotH, 5.0);
}

float distribution(float NdotH, float a)
{
	float NdotH2 = NdotH * NdotH;
	float a2 = a * a;
	float denom = (NdotH2 * (a2 - 1) + 1);
	denom = _PI_ * denom * denom;
	float D = a2 / max(denom, 0.001);
	return D;
}

float geometric(float NH, float NO, float HO, float NI)
{
	float G = 2.0 * NH * min(NI, NO) / (0.001 + HO);
	return min(1.0, G);
}

vec3 cook_torrance
(
	const in vec3 pSurfToEye,
	const in vec3 pSurfToLight,
	const in vec3 pPos,
	const in vec3 pNormal,
	const in vec3 pAlbedo,
	const in vec2 pMetallic,
	const in vec3 pEmission
)
{
	vec3 halfVector = normalize(pSurfToEye + pSurfToLight);

	float NdotL = clamp(dot(pNormal, pSurfToLight), 0.0, 1.0);
	float NdotV = clamp(dot(pNormal, pSurfToEye), 0.0, 1.0);
	float NdotH = clamp(dot(pNormal, halfVector), 0.0, 1.0);
	float HdotV = clamp(dot(halfVector, pSurfToEye), 0.0, 1.0);
	float HdotL = clamp(dot(halfVector, pSurfToLight), 0.0, 1.0);

	float metallic = pMetallic.x;
	float shininess = pMetallic.y;

	vec3 F = fresnel(pAlbedo, max(dot(halfVector, pSurfToEye), 0.0), metallic, shininess);
	float D = distribution(NdotH, shininess);
	float G = geometric(NdotH, NdotV, HdotV, NdotL);
	vec3 ks = (F * G * D) / max((4.0 * NdotL * NdotV), 0.0001);
	vec3 kd = (pAlbedo / _PI_) * (1.0 - F) * (1.0 - metallic);
	float dist = distance(LightPos, pPos);

	return (ks + kd) * LightColor * NdotL + pEmission;
}

void main()
{
	vec3 albedo = u_Diffuse.rgb;
	if(u_HasAlbedo)
		albedo = texture(AlbedoTexture, v_TextCoords).rgb;

	vec3 emission = u_EmitColor.rgb;
	if(u_HasEmissive)
		emission = texture(EmissiveTexture, v_TextCoords).rgb;

	float gloss = u_Shininess;
	float metallic = u_Metallicness;
	//float ao = (u_AmbientColor.r + u_AmbientColor.g + u_AmbientColor.b) / 3;
	//float reflectance = 0.5;

	if(u_HasMetallic)
	{
		vec4 mask = texture(MetallicTexture, v_TextCoords);
		metallic = mask.r;
		gloss = 1.0 - mask.a;
	}

	vec3 normal = v_TBN[2];
	if(u_HasNormal)
	{
		vec3 nmap = texture(NormalTexture, v_TextCoords).rgb;
		nmap = nmap * 2.0 - 1.0;
		normal = normalize(v_TBN * nmap);
	}

	vec3 surfToEye = normalize(u_CameraPos - v_Position_WCS);
	vec3 surfToLight = normalize(LightPos - v_Position_WCS);

	vec3 color = cook_torrance(surfToEye, -LightDir, v_Position_WCS, normal,
			albedo, vec2(metallic, gloss), vec3(emission.r, emission.g, emission.b));
	o_Color = vec4(color, 1.0);
}
