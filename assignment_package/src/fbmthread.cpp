#include "fbmthread.h"
#include <iostream>

using namespace std;

FBMThread::FBMThread(Chunk* ch, int _x, int _z, int _xChunk, int _zChunk, float (*fbmPtr)(float, float), BlockType top, QMutex* m)
    : c(ch), x(_x), z(_z), xChunk(_xChunk), zChunk(_zChunk), topBlock(top), mutex(m)
{
    fbm = fbmPtr;
}

void FBMThread::run() {
    float height = fbm(xChunk*16 + x, zChunk*16 + z);
    switch(topBlock) {
    case SAND:
        height = 128 + height * 10;
        break;
    case GRASS:
        height = 128 + height * 12;
        break;
    case SNOW:
        height = 128 + height * 14;
        break;
    case STONE:
        height = 128 + height * 16;
    }

    if (height < 128) {
        height = 128.f;
    }
    else if (height > 256) {
        height = 256.f;
    }
    for(int y = 0; y < 256; y++) {
        if(y < height) {
            if(y == ceil(height) - 1) {
                *(c->getBlockTypeRef(x, y, z)) = topBlock;
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
