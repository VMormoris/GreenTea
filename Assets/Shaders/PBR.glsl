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
layout(location = 1) out uint o_ObjectID;

const float _PI_ = 3.14159;


uniform vec4 u_Diffuse;
uniform vec4 u_Specular;
uniform vec4 u_Ambient;
uniform float u_Shininess;

uniform bool u_HasTexture;

uniform bool u_HasNormal;
uniform bool u_HasMask;
uniform bool u_HasEmissive;

uniform int u_LightType;
uniform vec4 u_LightColor;
uniform vec3 u_LightPos;
uniform vec3 u_LightDir;
uniform float u_Umbra;
uniform float u_Penumbra;
uniform mat4 u_LightProjectionMatrix;
uniform float u_ConstantBias;

uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;

uniform uint u_ID;

uniform vec2 u_ViewportSize;

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform sampler2D MaskTexture;
uniform sampler2D EmissiveTexture;
uniform sampler2D ShadowmapTexture;

in vec2 v_TextCoords;
in vec3 v_Position_WCS;
in vec3 v_Normal;
in mat3 v_TBN;


float compute_spotlight(const in vec3 pSurfToLight)
{
	float cos_umbra = cos(radians(0.5 * u_Umbra));
	float cos_penumbra = cos(radians(0.5 * u_Penumbra));
	float spoteffect = 1;
	float angle_vertex_spot_dir = dot(-pSurfToLight, u_LightDir);

	if (angle_vertex_spot_dir > cos_umbra) 
		spoteffect = 1;
	else if(angle_vertex_spot_dir >= cos_penumbra) 
		spoteffect = smoothstep(cos_penumbra, cos_umbra, angle_vertex_spot_dir);
	else
		spoteffect = 0;
	
	return spoteffect;
}

float shadow_pcf2x2_weighted(vec3 light_space_xyz)
{
	ivec2 shadow_map_size = textureSize(ShadowmapTexture, 0);
	float xOffset = 1.0 / shadow_map_size.x;
    float yOffset = 1.0 / shadow_map_size.y;

	// compute the weights of the neighboring pixels
	vec2 uv = light_space_xyz.xy - vec2(xOffset, yOffset);
	float u_ratio = mod(uv.x, xOffset) / xOffset;
	float v_ratio = mod(uv.y, yOffset) / yOffset;
	float u_opposite = 1 - u_ratio;
	float v_opposite = 1 - v_ratio;

	// compute the distance with a small bias
	float z = light_space_xyz.z - u_ConstantBias;

	// compute the shadow percentage
	float bottomLeft = (texture(ShadowmapTexture, uv).r > z) ? u_opposite : 0.0;
	float bottomRight = (texture(ShadowmapTexture, uv + vec2(xOffset, 0)).r > z) ? u_ratio : 0.0; 
	float topLeft = (texture(ShadowmapTexture, uv + vec2(0, yOffset), 0).r > z) ? u_opposite : 0.0;
	float topRight = texture(ShadowmapTexture, uv + vec2(xOffset, yOffset)).r > z ? u_ratio : 0.0;
	float factor = (bottomLeft + bottomRight) * v_opposite + (topLeft + topRight) * v_ratio;

    return factor;
}

// 1 sample per pixel
float shadow(vec3 pwcs)
{
	// project the pwcs to the light source point of view
	vec4 plcs = u_LightProjectionMatrix * vec4(pwcs, 1.0);
	// perspective division
	plcs /= plcs.w;

	// convert from [-1 1] to [0 1]
	plcs.xy = (plcs.xy + 1) * 0.5;

	// check that we are inside light clipping frustum
	if (plcs.x < 0.0) return 0.0;
	if (plcs.y < 0.0) return 0.0;
	if (plcs.x > 1.0) return 0.0;
	if (plcs.y > 1.0) return 0.0;

	// set scale of light space z vaule to [0, 1]
	plcs.z = 0.5 * plcs.z + 0.5;
	
	// sample shadow map with antialiasing
	float sum = shadow_pcf2x2_weighted(plcs.xyz);
	float count = 1.0;
	
	const float xOffset = 1.0 / u_ViewportSize.x;
	const float yOffset = 1.0 / u_ViewportSize.y;

	if((plcs.x + xOffset < 1.0) && (plcs.y - yOffset > 0.0))
	{
		sum += shadow_pcf2x2_weighted(plcs.xyz + vec3(xOffset, -yOffset, 0.0));
		count += 1.0;
	}
	if((plcs.x - xOffset > 0.0) && (plcs.y + yOffset < 1.0))
	{
		sum += shadow_pcf2x2_weighted(plcs.xyz + vec3(-xOffset, yOffset, 0.0));
		count += 1.0;
	}
	return sum/count;
	//return shadow_nearest(plcs.xyz);
	//return shadow_pcf2x2_weighted(plcs.xyz);
	//return shadow_pcf2x2_mean(plcs.xyz);
}

vec3 fresnel(
	const in vec3 diffColor,
	const in vec3 reflectance,
	const in float VdotH,
	const in float metallic)
{
	const vec3 f0 = mix(reflectance, diffColor * reflectance, metallic);
	float u = 1.0 - VdotH;
	float u5 = (u * u) * (u * u) * u;
	return min(vec3(1.0), f0  + (vec3(1.0) - f0) * u5);
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

vec3 cook_torrance(
	const in vec3 pSurfToEye,
	const in vec3 pSurfToLight,
	const in vec3 pPos,
	const in vec3 pNormal,
	const in vec3 pAlbedo,
	const in vec4 pMask,
	const in vec3 pEmission)
{
	vec3 halfVector = normalize(pSurfToEye + pSurfToLight);

	float NdotL = clamp(dot(pNormal, pSurfToLight), 0.0, 1.0);
	float NdotV = clamp(dot(pNormal, pSurfToEye), 0.0, 1.0);
	float NdotH = clamp(dot(pNormal, halfVector), 0.0, 1.0);
	float HdotV = clamp(dot(halfVector, pSurfToEye), 0.0, 1.0);
	float HdotL = clamp(dot(halfVector, pSurfToLight), 0.0, 1.0);

	float metallic = pMask.r;
	float ao = pMask.g;
	vec3 reflectance = vec3(pMask.b);
	float alpha = pMask.a;

	vec3 F = fresnel(pAlbedo, reflectance, max(dot(halfVector, pSurfToEye), 0.0), metallic);
	float D = distribution(NdotH, alpha);
	float G = geometric(NdotH, NdotV, HdotV, NdotL);
	vec3 ks = (F * G * D) / max((4.0 * NdotL * NdotV), 0.0001);
	vec3 kd = (pAlbedo / _PI_) * (1.0 - F) * (1.0 - metallic);
	float dist = distance(u_LightPos, pPos);

	if(u_LightType == 0)//Directional light
		return (ks + kd) * u_LightColor.rgb * NdotL + pEmission;
	else//Spot light
		return (ks + kd) * (u_LightColor.rgb / pow(dist, 2)) * NdotL + pEmission;
}

void main()
{
	vec3 albedo = u_Diffuse.rgb;
	if(u_HasTexture)
		albedo *= texture(DiffuseTexture, v_TextCoords).rgb;

	vec3 emission = u_Ambient.rgb;
	if(u_HasEmissive)
		emission *= texture(EmissiveTexture, v_TextCoords).rgb;

	float reflectance = (u_Specular.r + u_Specular.g + u_Specular.b) / 3;
	float gloss = u_Shininess;
	float metallic = u_Specular.r;
	float ao = 0.0;

	if(u_HasMask)
	{
		vec4 mask = texture(MaskTexture, v_TextCoords);
		metallic *= mask.r;
		ao = mask.g;
		reflectance = mask.b;
		gloss *= 1.0 - mask.a;
	}

	vec3 normal = v_TBN[2];
	if(u_HasNormal)
	{
		vec3 nmap = texture(NormalTexture, v_TextCoords).rgb;
		nmap = nmap * 2.0 - 1.0;
		normal = normalize(v_TBN * nmap);
	}

	vec3 surfToEye = normalize(u_CameraPos - v_Position_WCS);
	vec3 surfToLight = normalize(u_LightPos - v_Position_WCS);
	float shadow_value = shadow(v_Position_WCS);

	vec3 color;
	switch(u_LightType){
	case 0://Directional light
		color = cook_torrance(surfToEye, -u_LightDir, v_Position_WCS, normal,
			albedo, vec4(metallic, ao, reflectance, gloss),
			vec3(emission.r, emission.g, emission.b));
		break;
	case 2://Spotlight
		color = cook_torrance(surfToEye, surfToLight, v_Position_WCS, normal,
			albedo, vec4(metallic, ao, reflectance, gloss),
			vec3(emission.r, emission.g, emission.b));
		float spotEffect = compute_spotlight(surfToLight);
		color *= spotEffect;
		break;
	}
	//color *= shadow_value;
	o_Color += vec4(color, 1.0);
	
	/*if(u_HasLight)
	{
		vec3 surfToLight = normalize(u_LightPos - v_Position_WCS);
		float spotEffect = compute_spotlight(surfToLight);
		float shadow_value = shadow(v_Position_WCS);

		vec3 color = cook_torrance(surfToEye, surfToLight, v_Position_WCS, 
			normal,
			albedo, vec4(metallic, ao, reflectance, gloss),
			vec3(emission.r, emission.g, emission.b));

		color *= spotEffect * shadow_value;

		o_Color = vec4(color, 1.0);
	}
	else
		o_Color = vec4(albedo, 1.0);*/

	o_ObjectID = u_ID;
}
