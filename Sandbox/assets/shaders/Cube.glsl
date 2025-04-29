// Cube shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 Normal;
out vec3 FragmentPosition;

void main()
{
    gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    FragmentPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
}

#type fragment
#version 330 core
  
uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform vec3 u_ViewPosition;

in vec3 Normal;
in vec3 FragmentPosition;

out vec4 color;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_LightPosition - FragmentPosition); 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_LightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(u_ViewPosition - FragmentPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * u_LightColor; 

    vec3 result = (ambient + diffuse + specular) * u_ObjectColor;
    color = vec4(result, 1.0);
}