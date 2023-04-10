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
out vec3 v_WorldPos;
out mat3 v_TBN;

void main(void)
{
	v_TBN = mat3(
		normalize(vec3(u_NormalMatrix * vec4(_tangent, 0.0))),
		normalize(vec3(u_NormalMatrix * vec4(_bitangent, 0.0))),
		normalize(vec3(u_NormalMatrix * vec4(_normal, 0.0))));

	v_TextCoords = _textCoords;
	v_WorldPos = vec3(u_ModelMatrix * vec4(_position, 1.0));
	gl_Position = u_EyeMatrix * vec4(v_WorldPos, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

const float _PI_ = 3.14159265359;
const vec3 u_LightPos[] = {
	vec3(-10.0,  10.0, 10.0),
	vec3( 10.0,  10.0, 10.0),
	vec3(-10.0, -10.0, 10.0),
	vec3( 10.0, -10.0, 10.0),
};
const vec3 u_LightColor = vec3(.0);

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

uniform sampler2D AlbedoTexture;
uniform sampler2D NormalTexture;
uniform sampler2D MetallicTexture;
uniform sampler2D RoughTexture;
uniform sampler2D OclussionTexture;

// IBL
uniform samplerCube IrradianceTexture;
uniform samplerCube PrefilterTexture;
uniform sampler2D LUTTexture;

in vec2 v_TextCoords;
in vec3 v_WorldPos;
in mat3 v_TBN;

float DistributionGGX(const vec3 N, const vec3 H, float roughness)
{
	const float a = roughness*roughness;
    const float a2 = a*a;
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = NdotH*NdotH;

    const float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = _PI_ * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	const float r = (roughness + 1.0);
    const float k = (r*r) / 8.0;

    const float nom   = NdotV;
    const float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(const vec3 N, const vec3 V, const vec3 L, float roughness)
{
    const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
    const float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    const float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, const vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, const vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
	vec3 albedo = u_Diffuse.rgb;
	if (u_HasAlbedo)
		albedo *= pow(texture(AlbedoTexture, v_TextCoords).rgb, vec3(2.2));

	vec3 normal = v_TBN[2];
	if (u_HasNormal)
	{
		vec3 nmap = texture(NormalTexture, v_TextCoords).rgb;
		nmap = nmap * 2.0 - 1.0;
		normal = normalize(v_TBN * nmap);
	}

	float metallicness = u_Metallicness;
	float roughness = u_Roughness;
	vec3 ao = u_AmbientColor.rgb;
	
	if (u_HasMetallic)
		metallicness *= texture(MetallicTexture, v_TextCoords).r;
	if (u_HasRough)
		roughness *= texture(RoughTexture, v_TextCoords).r;
	if (u_HasOcclusion)
		ao *= texture(OclussionTexture, v_TextCoords).rgb;

	const vec3 N = v_TBN[2];
	const vec3 V = normalize(u_CameraPos - v_WorldPos);
	const vec3 R = reflect(-V, N); 

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    const vec3 F0 = mix(vec3(0.04), albedo, metallicness);
	
	vec3 Lo = vec3(0.0);
	// Reflection equation
	for (int i = 0; i < 4; i++)
	{
		const vec3 L = normalize(u_LightPos[i] - v_WorldPos);
		const vec3 H = normalize(V + L);
        const vec3 radiance = u_LightColor;

		// Cook-Torrance BRDF
		const float NDF = DistributionGGX(N, H, roughness);   
		const float G   = GeometrySmith(N, V, L, roughness);      
		const vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

		const vec3 numerator    = NDF * G * F; 
		const float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		const vec3 specular = numerator / denominator;

		// kS is equal to Fresnel
		vec3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
		vec3 kD = vec3(1.0) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
		kD *= 1.0 - metallicness;

		// scale light by NdotL
		const float NdotL = max(dot(N, L), 0.0);

		Lo += (kD * albedo / _PI_ + specular) * radiance * NdotL;
	}

	// ambient lighting (we now use IBL as the ambient term)
	const vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    const vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallicness;	  
    const vec3 irradiance = texture(IrradianceTexture, N).rgb;
    const vec3 diffuse = irradiance * albedo;
    
	// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    const vec3 prefilteredColor = textureLod(PrefilterTexture, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    const vec2 brdf  = texture(LUTTexture, vec2(max(dot(N, V), 0.0), roughness)).rg;
    const vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    const vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;

	// HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    color = pow(color, vec3(1.0/2.2));
	o_Color = vec4(color, 1.0);
}
