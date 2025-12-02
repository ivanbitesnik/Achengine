// Basic Water color shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

const float pi = 3.14159;
uniform int u_NumWaves;
uniform float u_Amplitude[8];
uniform float u_Wavelength[8];
uniform float u_Speed[8];
uniform vec2 u_Direction[8];
uniform float u_Time;

out vec3 position;
out vec3 worldNormal;
out vec3 eyeNormal;

float wave(int i, float x, float y) {
    float frequency = 2*pi/u_Wavelength[i];
    float phase = u_Speed[i] * frequency;
    float theta = dot(u_Direction[i], vec2(x, y));
    return u_Amplitude[i] * sin(theta * frequency + u_Time * phase);
}

float waveHeight(float x, float y) {
    float height = 0.0;
    for (int i = 0; i < u_NumWaves; ++i)
        height += wave(i, x, y);
    return height;
}

float dWavedx(int i, float x, float y) {
    float frequency = 2*pi/u_Wavelength[i];
    float phase = u_Speed[i] * frequency;
    float theta = dot(u_Direction[i], vec2(x, y));
    float A = u_Amplitude[i] * u_Direction[i].x * frequency;
    return A * cos(theta * frequency + u_Time * phase);
}

float dWavedy(int i, float x, float y) {
    float frequency = 2*pi/u_Wavelength[i];
    float phase = u_Speed[i] * frequency;
    float theta = dot(u_Direction[i], vec2(x, y));
    float A = u_Amplitude[i] * u_Direction[i].y * frequency;
    return A * cos(theta * frequency + u_Time * phase);
}

vec3 waveNormal(float x, float y) {
    float dx = 0.0;
    float dy = 0.0;
    for (int i = 0; i < u_NumWaves; ++i) {
        dx += dWavedx(i, x, y);
        dy += dWavedy(i, x, y);
    }
    vec3 n = vec3(-dx, -dy, 1.0);
    return normalize(n);
}

void main()
{
	vec4 pos = u_Transform * vec4(a_Position, 1.0);
    pos.z += waveHeight(pos.x, pos.y);
    position = pos.xyz / pos.w;
    worldNormal = waveNormal(pos.x, pos.y);
    eyeNormal = a_Normal * worldNormal;
    gl_Position = u_ViewProjection * pos;
}

#type fragment
#version 330 core

in vec3 position;
in vec3 worldNormal;
in vec3 eyeNormal;

uniform vec3 u_ViewPosition;
uniform samplerCube u_EnvMap;

void main() {
     vec3 eye = normalize(u_ViewPosition - position);
     vec3 r = reflect(eye, worldNormal);
     vec4 color = textureCube(u_EnvMap, r);
     color.a = 0.5;
     gl_FragColor = color;
}