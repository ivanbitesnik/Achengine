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
  
struct Material {
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
  
uniform Material u_Material;
uniform Light u_Light;
uniform vec3 u_ViewPosition;

in vec3 Normal;
in vec3 FragmentPosition;

out vec4 color;

void main()
{
    vec3 ambient = u_Light.ambient * u_Material.ambient;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(u_Light.position - FragmentPosition); 

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = u_Light.diffuse * (diff * u_Material.diffuse);

    vec3 viewDir = normalize(u_ViewPosition - FragmentPosition);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
    vec3 specular = u_Light.specular * (spec * u_Material.specular); 

    vec3 result = (ambient + diffuse + specular) * u_Material.color;
    color = vec4(result, 1.0);
}