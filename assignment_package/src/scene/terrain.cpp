#include <scene/terrain.h>
#include <scene/cube.h>
#include <random>
#include <iostream>
#include <math.h>

using namespace glm;
using namespace std;

Terrain::Terrain() : dimensions(64, 256, 64)
{}

float rand(vec2 n) {
    return (fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453));
}


float interpNoise2D(float x, float y) {
    float intX = floor(x);
    float fractX = fract(x);
    float intY = floor(y);
    float fractY = fract(y);

    float v1 = rand(vec2(intX, intY));
    float v2 = rand(vec2(intX + 1, intY));
    float v3 = rand(vec2(intX, intY + 1));
    float v4 = rand(vec2(intX + 1, intY + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);

    return mix(i1, i2, fractY);
}

float fbm(float x, float y) {
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 0; i< octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq, y * freq) * amp;
    }

    return total;
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    // TODO: Make this work with your new block storage!
    return m_blocks[x][y][z];
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    // TODO: Make this work with your new block storage!
    m_blocks[x][y][z] = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            float height = fbm(x, z);
//            std::cout << height << std::endl;
            height = 116 + height*10;

            if (height < 128) {
                height = 128.f;
            }
            else if (height > 256) {
                height = 256.f;
            }
//            for(int y = 127; y < 256; ++y)
//            {
//                if(y <= 128)
//                {
//                    if((x + z) % 2 == 0)
//                    {
//                        m_blocks[x][y][z] = STONE;
//                    }
//                    else
//                    {
//                        m_blocks[x][y][z] = DIRT;
//                    }
//                }
//                else
//                {
//                    m_blocks[x][y][z] = EMPTY;
//                }
//            }
            for(int y = 0; y < height; y++) {
                if(y == ceil(height) - 1) {
                    setBlockAt(x, y, z, GRASS);
                }
                else if(y >= 128) {
                    setBlockAt(x, y, z, DIRT);
                }
                else {
                    setBlockAt(x, y, z, STONE);
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        m_blocks[x][129][0] = GRASS;
        m_blocks[x][130][0] = GRASS;
        m_blocks[x][129][63] = GRASS;
        m_blocks[0][130][x] = GRASS;
    }
    for(int y = 129; y < 140; ++y)
    {
        m_blocks[32][y][32] = GRASS;
    }
}


