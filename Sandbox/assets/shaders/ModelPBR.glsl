// Model PBR-style shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 v_WorldPos;
out vec2 v_TexCoords;
out mat3 v_TBN;

void main()
{
    vec4 worldPos = u_Transform * vec4(a_Position, 1.0);
    gl_Position = u_ViewProjection * worldPos;

    mat3 normalMatrix = mat3(transpose(inverse(u_Transform)));
    vec3 N = normalize(normalMatrix * a_Normal);
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 B = normalize(normalMatrix * a_Bitangent);

    v_WorldPos = worldPos.xyz;
    v_TexCoords = a_TexCoords;
    v_TBN = mat3(T, B, N);
}

#type fragment
#version 330 core

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float roughness;
    float metallic;
    float ao;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

const int MAX_LIGHTS = 16;
const float PI = 3.14159265359;

uniform Material u_Material;
uniform int u_LightCount;
uniform Light u_Lights[MAX_LIGHTS];
uniform vec3 u_ViewPosition;
uniform bool u_HasDiffuseMap;
uniform bool u_HasSpecularMap;
uniform bool u_HasNormalMap;

in vec3 v_WorldPos;
in vec2 v_TexCoords;
in mat3 v_TBN;

out vec4 color;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.0001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.0001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    vec3 albedo = u_HasDiffuseMap ? pow(texture(u_Material.diffuse, v_TexCoords).rgb, vec3(2.2)) : vec3(0.8);
    float roughness = clamp(u_Material.roughness, 0.05, 1.0);
    float metallic = clamp(u_Material.metallic, 0.0, 1.0);
    float ao = clamp(u_Material.ao, 0.0, 1.0);

    vec3 N = normalize(v_TBN[2]);
    if (u_HasNormalMap)
    {
        vec3 tangentNormal = texture(u_Material.normal, v_TexCoords).xyz * 2.0 - 1.0;
        N = normalize(v_TBN * tangentNormal);
    }

    vec3 V = normalize(u_ViewPosition - v_WorldPos);

    vec3 specularSample = u_HasSpecularMap ? texture(u_Material.specular, v_TexCoords).rgb : vec3(0.0);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    F0 = mix(F0, specularSample, u_HasSpecularMap ? 0.5 : 0.0);

    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_LightCount && i < MAX_LIGHTS; ++i)
    {
        vec3 lightVec = u_Lights[i].position - v_WorldPos;
        float distance = length(lightVec);
        vec3 L = normalize(lightVec);
        vec3 H = normalize(V + L);

        float attenuation = 1.0 / (u_Lights[i].constant + u_Lights[i].linear * distance + u_Lights[i].quadratic * distance * distance);
        vec3 radiance = (u_Lights[i].diffuse + u_Lights[i].specular) * attenuation;

        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = numerator / max(denominator, 0.0001);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= (1.0 - metallic);

        float NdotL = max(dot(N, L), 0.0);
        vec3 diffuse = (kD * albedo / PI) * NdotL;

        vec3 ambient = u_Lights[i].ambient * albedo * ao * attenuation;
        Lo += ambient + (diffuse + specular) * radiance * NdotL;
    }

    vec3 mapped = Lo / (Lo + vec3(1.0));
    vec3 gammaCorrected = pow(mapped, vec3(1.0 / 2.2));
    color = vec4(gammaCorrected, 1.0);
}
