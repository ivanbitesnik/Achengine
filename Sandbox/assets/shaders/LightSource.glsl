// Light source shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core
out vec4 FragColor;

struct Light
{
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light u_Light;

void main()
{
    vec3 result = (u_Light.ambient + u_Light.diffuse + u_Light.specular) * u_Light.color;
    FragColor = vec4(result, 1.0);
}