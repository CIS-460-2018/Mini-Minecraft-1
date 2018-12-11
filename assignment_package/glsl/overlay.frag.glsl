#version 150

in vec4 fs_Col;
in vec2 fs_UV;
uniform sampler2D u_RenderedTexture;
uniform int u_Mode;
uniform int u_Time;

out vec4 out_Col;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = .5;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}


void main(void)
{
    vec3 diffuseColor = texture(u_RenderedTexture, fs_UV).rgb; // pixel color from texture
    if (u_Mode > 0) {
        vec2 uv = fs_UV;
        float offset = 0.5 * sin((u_Time / 50.0f));
        uv += fbm(fs_UV * offset);
        diffuseColor = texture(u_RenderedTexture, uv).rgb;
        if (u_Mode == 1) {
            diffuseColor += vec3(0, 0, diffuseColor.z * 0.7);
        } else if (u_Mode == 2) {
            diffuseColor += vec3(diffuseColor.x * 0.7, 0, 0);
        }
        out_Col = vec4(diffuseColor, 0.3);
    }
    out_Col = vec4(diffuseColor, 1);
}
