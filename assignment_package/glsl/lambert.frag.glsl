#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

//uniform vec4 u_Color; // The color with which to render this instance of geometry.
uniform sampler2D u_RenderedTexture;
uniform int u_Time;
// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Nor;
in vec4 fs_LightVec;
uniform vec4 u_ViewVec;
uniform vec4 u_Player;
//in vec4 fs_Col;
in vec2 fs_UV;
in float fs_Cos;
in vec4 fs_Pos;
in float fs_Animate;
out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.

void main()
{
    // Material base color (before shading)
        vec4 diffuseColor = texture(u_RenderedTexture, fs_UV);
        if (fs_Animate != 0.0f) {
            if (u_Time % 80 != 0) {
                int amount = u_Time % 80;
                float incr = 1.0 / 256.0f;
                if (fs_UV.x + amount * incr > 1) {
                    amount -= 32;
                    if (fs_UV.x + amount * incr > 1) {
                        amount -= 48;
                        diffuseColor = texture(u_RenderedTexture, vec2(fs_UV.x + amount * incr, fs_UV.y));
                    } else {
                        diffuseColor = texture(u_RenderedTexture, vec2(fs_UV.x + amount * incr, fs_UV.y - 1.0 / 16.0f));
                    }
                } else {
                    diffuseColor = texture(u_RenderedTexture, vec2(fs_UV.x + (amount * incr), fs_UV.y));
                }
            }
        }
        //out_Col = texture(u_RenderedTexture, fs_UV);
        // Calculate the diffuse term for Lambert shading
        float diffuseTerm = dot(normalize(fs_Nor), fs_LightVec);
        // Avoid negative lighting values
        diffuseTerm = clamp(diffuseTerm, 0, 1);

        vec3 halfVec = normalize((fs_LightVec - u_ViewVec).xyz);

        float ambientTerm = 0.2;

        float lightIntensity = 0.8 * diffuseTerm + ambientTerm;   //Add a small float value to the color multiplier
                                                            //to simulate ambient lighting. This ensures that faces that are not
                                                            //lit by our point light are not completely black.

        float specularIntensity = max(pow(dot(vec4(halfVec, 0) / 2.0, fs_Nor), fs_Cos), 0);
        float dist = length(vec4(fs_Pos.x, 0, fs_Pos.z, 0) - vec4(u_Player.x, 0, u_Player.z, 0));
        const vec3 fogColor = vec3(0.5, 0.5,0.5);
        float fogFactor = (150 - dist)/(50);
        fogFactor = clamp( fogFactor, 0.0, 1.0 );
        vec3 lightColor = diffuseColor.rgb * lightIntensity + vec3(specularIntensity);
        vec3 finalColor = mix(fogColor, lightColor, fogFactor);

        // Compute final shaded color
        out_Col = vec4(finalColor, diffuseColor.a);
}
