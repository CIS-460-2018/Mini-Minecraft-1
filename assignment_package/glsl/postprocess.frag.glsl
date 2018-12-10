#version 150

in vec2 fs_UV;

out vec3 color;

uniform sampler2D u_RenderedTexture;
uniform int u_Time;
uniform ivec2 u_Dimensions;

void main()
{
    float PI = 3.14159265358979323846; // Pi constant
    float E = 2.71828; // e constant
    float red = 0; // variable to store sum of reds
    float green = 0; // variable to store sum of greens
    float blue = 0; // variable to store sum of blues
    float x = fs_UV.x; // x coordinate of fs_UV
    float y = fs_UV.y; // y coordinate of fs_UV
    float xIncr = 1.0 / u_Dimensions.x; // space between pixels in x-axis
    float yIncr = 1.0 / u_Dimensions.y; // space between pixels in y-axis
    float xStart = x - 5 * xIncr; // starting x-coordinate
    float xCurr = xStart; // set current x to xStart
    float yCurr = y - 5 * yIncr; // starting y-coordinate
    float totalWeight = 0; // variable to store the sum of weights
    for (int i = 0; i < 11; i++) {
        // if current pixel coordinate is out of bounds, skip it
        if (yCurr < 0 || yCurr > u_Dimensions.y) {
            yCurr += yIncr;
            xCurr = xStart;
            continue;
        }
        for (int j = 0; j < 11; j++) {
            // if current pixel coordinate is out of bounds, skip it
            if (xCurr < 0 || xCurr > u_Dimensions.x) {
                xCurr += xIncr;
                continue;
            }
            float weight = (1 / (2 * PI * pow(9, 2))) * pow(E, - (xCurr * xCurr + yCurr + yCurr) / (2 * pow(9, 2))); // calculate weight
            vec3 neighborCol = texture(u_RenderedTexture, vec2(xCurr, yCurr)).rgb; // color of the current neighbor pixel
            red += (weight * neighborCol.x);
            green += (weight * neighborCol.y);
            blue += (weight * neighborCol.z);
            totalWeight += weight;
            xCurr += xIncr;
        }
        yCurr += yIncr;
        xCurr = xStart;
    }

    color = vec3(red, green, blue) / totalWeight;
}
