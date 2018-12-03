#include <scene/terrain.h>
#include <scene/cube.h>
#include <random>
#include <iostream>
#include <math.h>
#include "vbothread.h"
#include <QThreadPool>

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
    float persistence = 0.45f;
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
    int64_t key = getKey(x, z, false);
    int originalX = x;
    int originalZ = z;
    x = abs(x)%16;
    z = abs(z)%16;
    if(originalX < 0 && x != 0) {
        x = 16 - x;
    }
    if(originalZ < 0 && z != 0) {
        z = 16 - z;
    }
    if(!chunkMap.contains(key)) {
        return EMPTY;
    }
    return chunkMap[key]->getBlockType(x, y, z);
}

int64_t Terrain::getKey(int x, int z, bool chunked) const {
    int64_t xz = 0xffffffffffffffff;
    int64_t chunkx;
    int64_t chunkz;
    if(chunked) {
        chunkx = x;
        chunkz = z;
    } else {
        float fx = floorf(x / 16.0);
        float fz = floorf(z / 16.0);
        chunkx = int64_t(fx);
        chunkz = int64_t(fz);
    }
    xz = (xz & (chunkx << 32)) | 0x00000000ffffffff;
    chunkz = chunkz | 0xffffffff00000000;
    xz = xz & chunkz;
    return xz;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    int64_t key = getKey(x, z, false);
    if(!chunkMap.contains(key)) {
        chunkMap.insert(key, new Chunk(context));
    }
    int originalX = x;
    int originalZ = z;
    x = abs(x)%16;
    z = abs(z)%16;
    if(originalX < 0 && x != 0) {
        x = 16 - x;
    }
    if(originalZ < 0 && z != 0) {
        z = 16 - z;
    }
    *(chunkMap[key]->getBlockTypeRef(x, y, z)) = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = x_boundary_start; x < x_boundary_end; ++x)
    {
        for(int z = z_boundary_start; z < z_boundary_end; ++z)
        {
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
}

void Terrain::createNewChunk(glm::vec3 position) {
    int64_t xz = getKey(position.x, position.z, false);
    int64_t zChunk = xz & 0x00000000ffffffff;
    if(zChunk & 0x0000000080000000) {
        zChunk = zChunk | 0xffffffff00000000;
    }
    int64_t xChunk = (xz >> 32);
    Chunk* c = new Chunk(context);
    for(int x = 0; x < 16; ++x)
    {
        for(int z = 0; z < 16; ++z)
        {
            float height = fbm(xChunk*16 + x, zChunk*16 + z);

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
                        *(c->getBlockTypeRef(x, y, z)) = GRASS;
                    }
                    else if(y >= 128) {
                        *(c->getBlockTypeRef(x, y, z)) = DIRT;
                    }
                    else {
                        *(c->getBlockTypeRef(x, y, z)) = STONE;
                    }
                } else {
                    *(c->getBlockTypeRef(x, y, z)) = EMPTY;
                }
            }
        }
    }
    pair<int, int> ints ((int)xChunk, (int)zChunk);
    pair<pair<int, int>, Chunk*> p(ints, c);
    chunksToAdd.push_back(p);
}

void Terrain::updateChunk(glm::vec3 position) {
    QMutex mutex;
    int64_t xz = getKey(position.x, position.z, false);
    Chunk* c = chunkMap[xz];
    c->destroy();
    int64_t zChunk = xz & 0x00000000ffffffff;
    if(zChunk & 0x0000000080000000) {
        zChunk = zChunk | 0xffffffff00000000;
    }
    int64_t xChunk = (xz >> 32);
    VBOThread* thread = new VBOThread(c, (int)xChunk, (int)zChunk, chunkMap, &mutex);
    QThreadPool::globalInstance()->start(thread);
    QThreadPool::globalInstance()->waitForDone();
    c->create();
}

void Terrain::updateScene() {
    QMutex mutex;
    for(int64_t xz: chunkMap.keys()) {
        Chunk* c = chunkMap[xz];
        c->destroy();
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = (xz >> 32);
        VBOThread* thread = new VBOThread(c, (int)xChunk, (int)zChunk, chunkMap, &mutex);
        QThreadPool::globalInstance()->start(thread);
    }
    QThreadPool::globalInstance()->waitForDone();
    for(int64_t xz: chunkMap.keys()) {
        Chunk* c = chunkMap[xz];
        c->create();
    }
}

void Terrain::addChunks() {
    QMutex mutex;
    for(pair<pair<int, int>, Chunk*> p: chunksToAdd) {
        VBOThread* thread = new VBOThread(p.second, p.first.first, p.first.second, chunkMap, &mutex);
        QThreadPool::globalInstance()->start(thread);
    }
    QThreadPool::globalInstance()->waitForDone();
    for(pair<pair<int, int>, Chunk*> p: chunksToAdd) {
        chunkMap[getKey(p.first.first, p.first.second, true)] = p.second;
    }
    chunksToAdd.clear();
}

bool Terrain::hasChunk(int x, int z) {
    int64_t key = getKey(x, z, false);
    return chunkMap.contains(key);
}
