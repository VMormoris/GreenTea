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

const float _PI_ = 3.14159265359;
const vec3 LightDir = vec3(0.0, 0.0, -1.0);
const vec3 LightPos = vec3(0.0, 0.0, 100.0);
const vec3 LightColor = vec3(3.0, 3.0, 3.0);

uniform vec4 u_Diffuse;
uniform vec4 u_EmitColor;
uniform vec4 u_AmbientColor;
uniform float u_Metallicness;
uniform float u_Roughness;

uniform bool u_HasAlbedo;
uniform bool u_HasNormal;
uniform bool u_HasMetallic;
uniform bool u_HasRough;
uniform bool u_HasEmissive;
uniform bool u_HasOcclusion;
uniform bool u_HasOpacity;

uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;

uniform vec2 u_ViewportSize;

uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D RoughTexture;
uniform sampler2D OclussionTexture;
uniform sampler2D OpacityTexture;
uniform sampler2D EmissiveTexture;

in vec2 v_TextCoords;
in vec3 v_Position_WCS;
in vec3 v_Normal;
in mat3 v_TBN;


vec3 frenselShlick(float cosTheta, vec3 f0) { return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }

float distributionGGX(vec3 N, vec3 H, float roughness)
{
	const float a = roughness * roughness;
	const float a2 = a * a;
	const float NdotH = max(dot(N, H), 0.0);
	const float NdotH2 = NdotH * NdotH;
	

	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = _PI_ * denom * denom;

	return a2 / denom;
}

float geometrySchlickGGX(float NdotV, float roughness)
{
	const float r = roughness + 1.0f;
	const float k = (r * r) / 8.0;

	const float denom = NdotV * (1.0 - k) + k;
	return NdotV / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	const float NdotV = max(dot(N, V), 0.0);
	const float NdotL = max(dot(N, L), 0.0);
	const float ggx2 = geometrySchlickGGX(NdotV, roughness);
	const float ggx1 = geometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

float geometric(float NH, float NO, float HO, float NI)
{
	float G = 2.0 * NH * min(NI, NO) / (0.001 + HO);
	return min(1.0, G);
}

vec3 cook_torrance
(
	const in vec3 V,
	const in vec3 N,
	const in vec3 L,
	const in vec3 Pos,
	const in vec3 Albedo,
	const in vec3 Emission,
	const in float Metallicness,
	const in float Roughness
)
{
	const vec3 H = normalize(V + L);
	const vec3 radiance = LightColor;

	const vec3 f0 = mix(vec3(0.04), Albedo, Metallicness);
	const float NDF = distributionGGX(N, H, Roughness);
	const float G = geometrySmith(N, V, L, Roughness);
	vec3 F = frenselShlick(max(dot(H,V), 0.0), f0);

	const vec3 ks = F;
	const vec3 kd = (vec3(1.0) - ks) * (1.0 - Metallicness);
	
	const vec3 numerator = NDF * G * F;
	const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
	const vec3 specular = numerator / denominator;

	const float NdotL = max(dot(N, L), 0.0);
	return (kd * Albedo / _PI_ + specular) * radiance * NdotL + Emission;
}

void main()
{
	vec3 albedo = u_Diffuse.rgb;
	vec3 normal = v_TBN[2];

	float roughness = u_Roughness;
	float metallicness = u_Metallicness;

	vec3 ao = u_AmbientColor.rgb;
	vec3 emission = u_EmitColor.rgb;

	if(u_HasAlbedo)
		albedo *= texture(AlbedoTexture, v_TextCoords).rgb;

	if(u_HasNormal)
	{
		vec3 nmap = texture(NormalTexture, v_TextCoords).rgb;
		nmap = nmap * 2.0 - 1.0;
		normal = normalize(v_TBN * nmap);
	}
	if(u_HasMetallic)
		metallicness *= texture(MetallicTexture, v_TextCoords).r;
	if(u_HasRough)
		roughness *= texture(RoughTexture, v_TextCoords).r;

	
	if(u_HasOcclusion)
		ao *= texture(OclussionTexture, v_TextCoords).rgb;
	vec3 ambient = vec3(0.03) * albedo * ao;

	if(u_HasEmissive)
		emission *= texture(EmissiveTexture, v_TextCoords).rgb;

	vec3 surfToEye = normalize(u_CameraPos - v_Position_WCS);

	vec3 color = cook_torrance(surfToEye, normal, -LightDir, v_Position_WCS,
			albedo, emission, metallicness, roughness);
	
	color += ambient; 
	o_Color = vec4(color, 1.0);
}
