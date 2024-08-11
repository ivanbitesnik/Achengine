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
    Normal = a_Normal;
}

#type fragment
#version 330 core
  
uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;

in vec3 Normal;
in vec3 FragmentPosition;

out vec4 color;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_LightColor;

    vec3 normX = normalize(vec3(Normal.x, 0.0, 0.0));
    vec3 normY = normalize(vec3(0.0, Normal.y, 0.0));
    vec3 normZ = normalize(vec3(0.0, 0.0, Normal.z));
    vec3 lightDir = normalize(FragmentPosition - u_LightPosition); 

    float diffX = max(dot(normX, lightDir), 0.0);
    float diffY = max(dot(normY, lightDir), 0.0);
    float diffZ = max(dot(normZ, lightDir), 0.0);

    float diff = diffZ;
    if (diffX != 0.0) diff = diffX;
    else if (diffY != 0.0) diff = diffY;

    vec3 diffuse = diff * u_LightColor;

    vec3 result = (ambient + diffuse) * u_ObjectColor;
    color = vec4(result, 1.0);
}