// Cube shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 Normal;
out vec3 FragmentPosition;
out vec2 TexCoords;
out mat3 TBN;

void main()
{
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    FragmentPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    mat3 normalMatrix = mat3(transpose(inverse(u_Transform)));
    vec3 N = normalize(normalMatrix * a_Normal);
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 B = normalize(normalMatrix * a_Bitangent);

    Normal = N;
    TBN = mat3(T, B, N);
    TexCoords = a_TexCoords;
}

#type fragment
#version 330 core
  
struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
    float shininess;
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
  
uniform Material u_Material;
uniform int u_LightCount;
uniform Light u_Lights[MAX_LIGHTS];
uniform vec3 u_ViewPosition;
uniform bool u_HasDiffuseMap;
uniform bool u_HasSpecularMap;
uniform bool u_HasNormalMap;

in vec3 Normal;
in vec3 FragmentPosition;
in vec2 TexCoords;
in mat3 TBN;

out vec4 color;

void main()
{
    vec3 diffuseTex = u_HasDiffuseMap ? vec3(texture(u_Material.diffuse, TexCoords)) : vec3(1.0);
    vec3 specularTex = u_HasSpecularMap ? vec3(texture(u_Material.specular, TexCoords)) : vec3(1.0);
    vec3 norm = normalize(Normal);
    if (u_HasNormalMap)
    {
        vec3 tangentNormal = texture(u_Material.normal, TexCoords).xyz * 2.0 - 1.0;
        norm = normalize(TBN * tangentNormal);
    }

    vec3 viewDir = normalize(u_ViewPosition - FragmentPosition);

    vec3 result = vec3(0.0);
    for (int i = 0; i < u_LightCount && i < MAX_LIGHTS; ++i)
    {
        vec3 lightVector = u_Lights[i].position - FragmentPosition;
        float distance = length(lightVector);
        vec3 lightDir = normalize(lightVector);
        float attenuation = 1.0 / (u_Lights[i].constant + u_Lights[i].linear * distance + u_Lights[i].quadratic * distance * distance);

        vec3 ambient = u_Lights[i].ambient * diffuseTex * attenuation;

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = u_Lights[i].diffuse * diff * diffuseTex * attenuation;

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
        vec3 specular = u_Lights[i].specular * spec * specularTex * attenuation;

        result += ambient + diffuse + specular;
    }

    color = vec4(result, 1.0);
}