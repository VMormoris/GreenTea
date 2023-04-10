#type vertex
#version 450 core

layout(location = 0) in vec3 _position;
layout(location = 1) in vec2 _textCoords;

out vec2 v_TextCoords;

void main(void)
{
    v_TextCoords = _textCoords;
    gl_Position = vec4(_position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
in vec2 v_TextCoords;

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

float GeometryShlickGGX(const float NdotV, const float roughness)
{
    const float a = roughness;
    const float k = (a * a) / 2.0;

    const float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

float GeometrySmith(const vec3 N, const vec3 V, const vec3 L, const float roughness)
{
    const float NdotV = max(dot(N, V), 0.0);
    const float NdotL = max(dot(N, L), 0.0);
    const float ggx2 = GeometryShlickGGX(NdotV, roughness);
    const float ggx1 = GeometryShlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec2 IntegrateBRDF(const float NdotV, const float roughness)
{
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);
    
    float A = 0.0;
    float B = 0.0;
    vec3 N = vec3(0.0, 0.0, 1.0);

    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; i++)
    {
        const vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        const vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        const vec3 L = normalize(2.0 * dot(V, H) * H - V);

        const float NdotL = max(L.z, 0.0);
        const float NdotH = max(H.z, 0.0);
        const float VdotH = max(dot(V, H), 0.0);

        if(NdotL > 0.0)
        {
            const float G = GeometrySmith(N, V, L, roughness);
            const float G_Vis = (G * VdotH) / (NdotH * NdotV);
            const float Fc = pow(1.0 - VdotH, 5.0);

            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float(SAMPLE_COUNT);
    B /= float(SAMPLE_COUNT);
    return vec2(A, B);
}

void main()
{		
	vec2 brdf = IntegrateBRDF(v_TextCoords.x, v_TextCoords.y);
	o_Color = vec4(brdf.x, brdf.y, 0.0, 1.0);
}