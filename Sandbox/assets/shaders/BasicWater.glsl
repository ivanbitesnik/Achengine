// Basic Water color shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;

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

// Calculates wave value and its derivative, 
// for the wave direction, location in space, wave frequency and time
vec2 wavedx(vec2 location, vec2 direction, float frequency, float timeshift) {
    float x = dot(direction, location) * frequency + timeshift;
    float wave = exp(sin(x) - 1.0);
    float dx = wave * cos(x);
    return vec2(wave, -dx);
}

// Calculates waves by summing octaves of various waves with various parameters
float getWaves(vec2 location) {
    float wavePhaseShift = length(location) * 0.1; // this is to avoid every octave having exactly the same phase everywhere
    float iter = 0.0; // this will help generating well distributed wave directions
    float weight = 1.0;// weight in final sum for the wave, this will change every iteration
    float sumOfValues = 0.0; // will store final sum of values
    float sumOfWeights = 0.0; // will store final sum of weights
    for(int i=0; i < u_NumWaves; i++) {
        // generate some wave direction that looks kind of random
        vec2 p = vec2(sin(iter), cos(iter));
        float frequency = 2*pi/u_Wavelength[i];
        float phase = u_Speed[i] * frequency;
        
        // calculate wave data
        vec2 res = wavedx(location, u_Direction[i], frequency, u_Time * phase + wavePhaseShift);

        // shift position around according to wave drag and derivative of the wave
        location += p * res.y * weight * 0.38;

        // add the results to sums
        sumOfValues += res.x * weight;
        sumOfWeights += weight;

        // modify next octave
        weight = mix(weight, 0.0, 0.2);

        // add some kind of random value to make next wave look random too
        iter += 1232.399963;
    }
    // calculate and return
    return sumOfValues / sumOfWeights;
}

void main()
{
	vec4 pos = u_Transform * vec4(a_Position, 1.0);
    pos.y += waveHeight(pos.x, pos.z);
    position = pos.xyz / pos.w;
    worldNormal = waveNormal(pos.x, pos.z);
    gl_Position = u_ViewProjection * pos;
}

#type fragment
#version 330 core

in vec3 position;
in vec3 worldNormal;

uniform vec3 u_ViewPosition;
uniform samplerCube u_EnvMap;

void main() {
     vec3 eye = normalize(u_ViewPosition - position);
     vec3 r = reflect(eye, worldNormal);
     vec4 color = textureCube(u_EnvMap, r);
     color.a = 0.5;
     gl_FragColor = color;
}