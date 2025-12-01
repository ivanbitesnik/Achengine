// Basic Water color shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform float u_Amplitude;
uniform float u_Frequency;
uniform float u_Phase;
uniform float u_Time;

out vec2 v_TexCoord;

void main()
{
	v_TexCoord = a_TexCoord;

	float wave = u_Amplitude * sin(u_Frequency + u_Time * u_Phase);
	vec3 pos = a_Position;
	pos.z += wave;
	gl_Position = u_ViewProjection * u_Transform * vec4(pos, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

void main()
{
	color = texture(u_Texture, v_TexCoord) * u_Color;
}