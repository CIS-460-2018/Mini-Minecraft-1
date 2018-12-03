#include "vbothread.h"

VBOThread::VBOThread(Chunk* ch, int x, int z, QHash<int64_t, Chunk*> map, QMutex* m)
    : c(ch), xChunk(x), zChunk(z), chunkMap(map), mutex(m)
{}

void VBOThread::createVertexPosNorCol() {
    c->c_vert_pos_nor_col.clear();
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 256; y++) {
            for(int z = 0; z < 16; z++) {
                BlockType t;
                if((t = c->getBlockType(x, y, z)) != EMPTY) {
                    glm::vec4 col;
                    switch(t) {
                    case DIRT:
                        col = (glm::vec4(121.f, 85.f, 58.f, 255.f) / 255.f);
                        break;
                    case GRASS:
                        col = (glm::vec4(95.f, 159.f, 53.f, 255.f) / 255.f);
                        break;
                    case STONE:
                        col = (glm::vec4(0.5f));
                        break;
                    }

                    // top
                    if(checkEmpty(x, y+1, z)) {
                        glm::vec4 normal = glm::vec4(0, 1, 0, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                    // bottom
                    if(checkEmpty(x, y-1, z)) {
                        glm::vec4 normal = glm::vec4(0, -1, 0, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                    //right
                    if(checkEmpty(x+1, y, z)) {
                        glm::vec4 normal = glm::vec4(1, 0, 0, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                    //left
                    if(checkEmpty(x-1, y, z)) {
                        glm::vec4 normal = glm::vec4(-1, 0, 0, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                    //front
                    if(checkEmpty(x, y, z+1)) {
                        glm::vec4 normal = glm::vec4(0, 0, 1, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                    //back
                    if(checkEmpty(x, y, z-1)) {
                        glm::vec4 normal = glm::vec4(0, 0, -1, 0);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                        c->c_vert_pos_nor_col.push_back(glm::vec4(x, y+1, z, 1.0f));
                        c->c_vert_pos_nor_col.push_back(normal);
                        c->c_vert_pos_nor_col.push_back(col);
                    }
                }
            }
        }
    }
    c->faces = c->c_vert_pos_nor_col.size()/12;
}

bool VBOThread::checkEmpty(int x, int y, int z) {
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

int64_t VBOThread::getKey(int x, int z, bool chunked) const {
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

void VBOThread::run() {
    createVertexPosNorCol();
}
