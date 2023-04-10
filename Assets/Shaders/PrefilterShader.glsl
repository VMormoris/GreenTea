#type vertex
#version 450 core

layout(location = 0) in vec3 _position;

uniform mat4 u_EyeMatrix;

out vec3 v_Pos;


void main(void)
{
	v_Pos = _position;
	gl_Position = u_EyeMatrix * vec4(_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
in vec3 v_Pos;

uniform float u_Roughness;
uniform samplerCube EnvironmentMap;

const float _PI_ = 3.14159265359;

float RadianceInverseVdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(const uint i, const uint N) { return vec2(float(i)/float(N), RadianceInverseVdC(i)); }

vec3 ImportanceSampleGGX(const vec2 Xi, const vec3 N, const float roughness)
{
    const float a = roughness * roughness;
    
    const float phi = 2.0 * _PI_ * Xi.x;
    const float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // From spherical to cartesian coordinates
    const vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // From tangent-space vector to world-space sample vector
    const vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 tangent = normalize(cross(up, N));
    const vec3 bitangent = cross(N, tangent);

    return normalize(vec3(tangent * H.x + bitangent * H.y + N * H.z));
}

float DistributionGGX(const vec3 N, const vec3 H, float roughness)
{
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float NdotH = max(dot(N, H), 0.0);
    const float NdotH2 = NdotH * NdotH;

    float denom = (NdotH * (a2 - 1.0) + 1.0);
    denom = _PI_ * denom * denom;

    return a2 / denom;
}

void main()
{		
	const vec3 N = normalize(v_Pos);
    const vec3 R = N;
    const vec3 V = R;

    const uint SAMPLE_COUNT = 1024;
    float totalWeight = 0.0;
    vec3 prefilteredColor = vec3(0.0);
    for(uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        const vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        const vec3 H = ImportanceSampleGGX(Xi, N, u_Roughness);
        const vec3 L = normalize(2.0 * dot(V, H) * H - V);

        const float NdotL = max(dot(N, L), 0.0);
        if(NdotL > 0.0)
        {
            const float D = DistributionGGX(N, H, u_Roughness);
            const float NdotH = max(dot(N, H), 0.0);
            const float HdotV = max(dot(H, V), 0.0);
            const float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

            const float resolution = textureSize(EnvironmentMap, 0).x;
            const float saTexel = 4.0 * _PI_ / (6.0 * resolution * resolution);
            const float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

            const float mipLevel = u_Roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
            prefilteredColor += texture(EnvironmentMap, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    o_Color = vec4(prefilteredColor, 1.0);
}