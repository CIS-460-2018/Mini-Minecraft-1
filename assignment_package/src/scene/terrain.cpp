#include <scene/terrain.h>
#include <scene/cube.h>
#include <random>
#include <iostream>
#include <math.h>

using namespace glm;
using namespace std;

Terrain::Terrain(OpenGLContext* c)
    : context(c), dimensions(64, 256, 64), x_boundary_start(0), y_boundary_start(0), z_boundary_start(0),
      x_boundary_end(64), y_boundary_end(256),
      z_boundary_end(64)
{}

Terrain::Terrain(OpenGLContext* c, int x_boundary_end, int y_boundary_end, int z_boundary_end)
    : context(c), dimensions(x_boundary_end, y_boundary_end, z_boundary_end),
      x_boundary_start(0), x_boundary_end(x_boundary_end), y_boundary_start(0), y_boundary_end(y_boundary_end),
      z_boundary_start(0), z_boundary_end(z_boundary_end)
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
    x /= 64.f; y /= 64.f;
    float total = 0;
    float persistence = 0.5f;
    int octaves = 8;

    for(int i = 1; i<= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq, y * freq) * amp;
    }

    return total;
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    return chunkMap[getKey(x, z, false)]->getBlockType(x%16, y, z%16);
}

int64_t Terrain::getKey(int x, int z, bool chunked) const {
    int64_t xz = 0xffffffffffffffff;
    int64_t chunkx;
    int64_t chunkz;
    if(chunked) {
        chunkx = x;
        chunkz = z;
    } else {
        chunkx = x/16;
        chunkz = z/16;
    }
    chunkx = (chunkx << 32) | 0x00000000ffffffff;
    chunkz = chunkz | 0xffffffff00000000;
    xz = chunkz & chunkx;
    return xz;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    int64_t key = getKey(x, z, false);
    if(!chunkMap.contains(key)) {
        chunkMap.insert(key, new Chunk(context));
    }
    *(chunkMap[key]->getBlockTypeRef(x%16, y, z%16)) = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = x_boundary_start; x < x_boundary_end; ++x)
    {
        for(int z = z_boundary_start; z < z_boundary_end; ++z)
        {
//             for(int y = 0; y < 256; ++y)
//             {
//                 if(y == 128 || y == 127 )
//                 {
//                     if((x + z) % 2 == 0)
//                     {
//                         setBlockAt(x, y, z, STONE);
//                     }
//                     else
//                     {
//                         setBlockAt(x, y, z, DIRT);
//                     }
//                 }
//                 else
//                 {
//                     setBlockAt(x, y, z, EMPTY);
//                 }
            float height = fbm(x, z);

            height = 128 + height * 32;
            //height = 116 + height * 10;

            if (height < 128) {
                height = 128.f;
            }
            else if (height > 256) {
                height = 256.f;
            }
            for(int y = 0; y < 256; y++) {
                if(y < height) {
                    if(y == ceil(height) - 1) {
                        setBlockAt(x, y, z, GRASS);
                    }
                    else if(y >= 128) {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else {
                        setBlockAt(x, y, z, STONE);
                    }
                } else {
                    setBlockAt(x, y, z, EMPTY);
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
        setBlockAt(32, y, 32, GRASS);
    }
}

void Terrain::updateScene() {
    for(int64_t xz: chunkMap.keys()) {
        Chunk* c = chunkMap[xz];
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = xz >> 32;

        createVertexPosNorUV(c, xChunk, zChunk);

        c->facesOpaque = c->c_vert_pos_nor_uv_opaque.size() / 12;
        c->facesTransparent = c->c_vert_pos_nor_uv_transparent.size() / 12;
    }
}

void Terrain::createVertexPosNorUV(Chunk* c, int xChunk, int zChunk) {
    c->c_vert_pos_nor_uv_opaque.clear();
    c->c_vert_pos_nor_uv_transparent.clear();
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 256; y++) {
            for(int z = 0; z < 16; z++) {
                BlockType t;
                if((t = c->getBlockType(x, y, z)) != EMPTY) {
                    float cosinePow = 0.0f;
                    float isAnimate = 0;
                    bool isTransparent = false;
                    uvIndicator = 0;
                    switch(t) {
                    case DIRT:
                        uvIndicator = 1;
                        cosinePow = 90.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case GRASS:
                        uvIndicator = 2;
                        cosinePow = 70.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case STONE:
                        uvIndicator = 3;
                        cosinePow = 30.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case WOOD:
                        uvIndicator = 4;
                        cosinePow = 90.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case LEAF:
                        uvIndicator = 5;
                        cosinePow = 70.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case BEDROCK:
                        uvIndicator = 6;
                        cosinePow = 50.0f;
                        isAnimate = 0;
                        isTransparent = false;
                        break;
                    case LAVA:
                        uvIndicator = 7;
                        cosinePow = 10.0f;
                        isAnimate = 1;
                        isTransparent = false;
                        break;
                    case WATER:
                        uvIndicator = 8;
                        cosinePow = 10.0f;
                        isAnimate = 1;
                        isTransparent = true;
                        break;
                    case ICE:
                        uvIndicator = 9;
                        cosinePow = 10.0f;
                        isAnimate = 0;
                        isTransparent = true;
                        break;
                    }

                    // top
                    if(checkEmpty(x, y+1, z, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(0); // get the top uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(0, 1, 0, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                    // bottom
                    if(checkEmpty(x, y-1, z, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(1); // get the bottom uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(0, -1, 0, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                    //right
                    if(checkEmpty(x+1, y, z, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(2); // get the bottom uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(1, 0, 0, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                    //left
                    if(checkEmpty(x-1, y, z, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(2); // get the side uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(-1, 0, 0, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                    //front
                    if(checkEmpty(x, y, z+1, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(2); // get the side uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(0, 0, 1, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                    //back
                    if(checkEmpty(x, y, z-1, c, xChunk, zChunk)) {
                        glm::vec2 uvPoint = getTexture(2); // get the side uv starting from top left vertex
                        glm::vec4 normal = glm::vec4(0, 0, -1, 0);
                        if (isTransparent) {
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_transparent.push_back(normal);
                            c->c_vert_pos_nor_uv_transparent.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        } else {
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y -= 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.x += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                            uvPoint.y += 1.0f / 16.0f;
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                            c->c_vert_pos_nor_uv_opaque.push_back(normal);
                            c->c_vert_pos_nor_uv_opaque.push_back(glm::vec4(uvPoint.x, uvPoint.y, cosinePow, isAnimate));
                        }
                    }
                }
            }
        }
    }
}

bool Terrain::checkEmpty(int x, int y, int z, Chunk* c, int xChunk, int zChunk) {
    if(x >= 0 && y >= 0 && z >= 0 && x < 16 && y < 256 && z < 16) {
        return c->getBlockType(x, y, z) == EMPTY;
    } else {
        if(x < 0) {
            if(chunkMap.contains(getKey(xChunk-1, zChunk, true))) {
                return chunkMap[getKey(xChunk-1, zChunk, true)]->getBlockType(15, y, z%16) == EMPTY;
            }
        } else if(y < 0) {
            return true; // under terrain, nothing there
        } else if(z < 0) {
            if(chunkMap.contains(getKey(xChunk, zChunk-1, true))) {
                return chunkMap[getKey(xChunk, zChunk-1, true)]->getBlockType(x%16, y, 15) == EMPTY;
            }
        } else if(x >= 16) {
            if(chunkMap.contains(getKey(xChunk+1, zChunk, true))) {
                return chunkMap[getKey(xChunk+1, zChunk, true)]->getBlockType(0, y, z%16) == EMPTY;
            }
        } else if(y >= 256) {
            return true;
        } else {
            if(chunkMap.contains(getKey(xChunk, zChunk+1, true))) {
                return chunkMap[getKey(xChunk, zChunk+1, true)]->getBlockType(x%16, y, 0) == EMPTY;
            }
        }
        return true;
    }
}

vec2 Terrain::getTexture(int faceNum)
{
    // if dirt
    if (uvIndicator == 1) {
        return glm::vec2(1.0f / 16.0f * 2, 1.0f);
    } else if (uvIndicator == 2) { // if grass
        // check if grass top
        if (faceNum == 0) {
            return glm::vec2(1.0f / 16.0f * 8, 1.0f / 16.0f * 14);
        } else if (faceNum == 1) { // check if grass bottom
            return glm::vec2(1.0f / 16.0f * 2, 1.0f);
        } else { // grass sides
            return glm::vec2(1.0f / 16.0f * 3, 1.0f);
        }
    } else if (uvIndicator == 3) { // if stone
        return glm::vec2(1.0f / 16.0f * 1, 1.0f);
    } else if (uvIndicator == 4) { // if wood
        // check if wood top or bottom
        if (faceNum == 0) {
        return glm::vec2(1.0f / 16.0f * 5, 1.0f / 16.0f * 15);
        } else { // wood sides
        return glm::vec2(1.0f / 16.0f * 4, 1.0f / 16.0f * 15);
        }
    } else if (uvIndicator == 5) { // if leaf
        return glm::vec2(1.0f / 16.0f * 5, 1.0f / 16.0f * 13);
    } else if (uvIndicator == 6) { // if bedrock
        return glm::vec2(1.0f / 16.0f * 1, 1.0f / 16.0f * 15);
    } else if (uvIndicator == 7) { // if lava
        return glm::vec2(1.0f / 16.0f * 13, 1.0f / 16.0f * 2);
    } else if (uvIndicator == 8) { // if water
        return glm::vec2(1.0f / 16.0f * 13, 1.0f / 16.0f * 4);
    } else { // if ice
        return glm::vec2(1.0f / 16.0f * 3, 1.0f / 16.0f * 12);
    }
}

