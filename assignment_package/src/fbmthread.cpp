#include "fbmthread.h"

FBMThread::FBMThread(Chunk* ch, int _x, int _z, int _xChunk, int _zChunk, float(*fbmPtr)(float, float), QMutex* m)
    : c(ch), x(_x), z(_z), xChunk(_xChunk), zChunk(_zChunk), mutex(m) {
    fbm = fbmPtr;
}

void FBMThread::run() {
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
