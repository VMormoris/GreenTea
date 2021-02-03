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

uniform bool u_HasLight;
uniform vec4 u_LightColor;
uniform vec3 u_LightPos;
uniform vec3 u_LightDir;
uniform float u_Umbra;
uniform float u_Penumbra;
uniform vec3 u_CameraPos;
uniform vec3 u_CameraDir;

uniform bool u_HasNormal;
uniform bool u_IsBump;


uniform mat4 u_LightProjectionMatrix;
uniform float u_ConstantBias;

uniform uint u_ID;

uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
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

	// sample shadow map
	//return shadow_nearest(plcs.xyz);
	return shadow_pcf2x2_weighted(plcs.xyz);
	//return shadow_pcf2x2_mean(plcs.xyz);
}

vec4 blinn_phong(const in vec3 pSurfToEye, const in vec3 pSurfToLight)
{
	vec3 normal = v_TBN[2];

	if(u_HasNormal && u_HasTexture)
	{
		vec3 nmap = texture(NormalTexture, v_TextCoords).rgb;

		if(u_IsBump)
		{
			float heigh_prev_U = textureOffset(NormalTexture, v_TextCoords, ivec2(1, 0)).r;
			float heigh_prev_V = textureOffset(NormalTexture, v_TextCoords, ivec2(0, 1)).r;
			normal = normal - v_TBN[0] * (nmap.r - heigh_prev_U) - v_TBN[1] * (nmap.r - heigh_prev_V);
		}
		else
		{
			nmap = nmap * 2.0 - 1.0;
			normal = normalize(v_TBN * nmap);
		}
	}

	vec3 halfVector = normalize(pSurfToEye + pSurfToLight);

	float NdotL = max(dot(normal, pSurfToLight), 0.0);
	float NdotH = max(dot(normal, halfVector), 0.0);

	vec4 albedo;
	if(u_HasTexture)
		albedo = texture(DiffuseTexture, v_TextCoords);// * u_Diffuse;
	else
		albedo = u_Diffuse;
	
	vec4 kd = albedo / _PI_;
	vec4 ks = u_Specular;

	float fn =
		((u_Shininess + 2) * (u_Shininess + 4)) /
		(8 * _PI_ * (u_Shininess + 1.0 / pow(2, u_Shininess / 2.0)));

	vec4 diffuse = kd * NdotL;
	vec4 specular = NdotL > 0.0 ? ks * fn * pow(NdotH, u_Shininess) : vec4(0.0);

	return (diffuse + specular) * u_LightColor + u_Ambient;
}

void main()
{
	if(u_HasLight)
	{
		vec3 surfToEye = normalize(u_CameraPos - v_Position_WCS);
		vec3 surfToLight = normalize(u_LightPos - v_Position_WCS);

		float shadow_value = shadow(v_Position_WCS);

		vec4 brdf = blinn_phong(surfToEye, surfToLight);
		float spotEffect =  compute_spotlight(surfToLight);
		float dist = distance(u_LightPos, v_Position_WCS);
		
		o_Color = vec4(shadow_value * spotEffect * brdf.rgb / pow(dist, 2.0), 1.0);
	}
	else
	{
		vec4 DiffuseColor = vec4(u_Diffuse.rgb, 1.0);
		if(u_HasTexture)
			DiffuseColor *= texture(DiffuseTexture, v_TextCoords);
		o_Color = DiffuseColor;
	}

	o_ObjectID = u_ID;
}
