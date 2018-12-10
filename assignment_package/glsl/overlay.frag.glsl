#version 150

in vec4 fs_Col;
in vec2 fs_UV;
uniform sampler2D u_RenderedTexture;
uniform int u_Mode;
uniform int u_Time;

out vec4 out_Col;


void main(void)
{
    vec3 diffuseColor = texture(u_RenderedTexture, fs_UV).rgb; // pixel color from texture
    if (u_Mode > 0) {
        if (u_Mode == 1) {
            diffuseColor *= 0.3;
            diffuseColor += vec3(0, 0, diffuseColor.z * 0.7);
        } else if (u_Mode == 2) {
            diffuseColor *= 0.3;
            diffuseColor += vec3(diffuseColor.x * 0.7, 0, 0);
        }
        /*float grey = diffuseColor.x * 0.21 + diffuseColor.y * 0.72 + diffuseColor.z * 0.07; // the grey color
        float distanceX = abs(fs_UV.x - 0.5); // x-distance
        float distanceY = abs(fs_UV.y - 0.5); // y-distance
        float distanceTotal = sqrt(distanceX * distanceX + distanceY * distanceY); // distance from screen center
        float actualDistance = distanceTotal - 0.2; // subtract the ellipse radius
        if (actualDistance > 0) {
            grey = clamp(grey - actualDistance, 0, grey);
        }
        out_Col = vec4(vec3(grey, grey, grey), 1);*/
    } else {
        //out_Col = vec4(diffuseColor, 1);
    }
    out_Col = vec4(diffuseColor, 1);
}
