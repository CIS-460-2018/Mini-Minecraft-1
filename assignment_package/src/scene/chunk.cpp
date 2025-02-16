#include "chunk.h"
bool Chunk::isOpaqueDrawn = false;

Chunk::Chunk(OpenGLContext* context) : Drawable(context) {}

Chunk::Chunk(OpenGLContext* context, BlockType blocks[16][256][16]): Drawable(context) {
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 256; y++) {
            for(int z = 0; z < 16; z++) {
                *getBlockTypeRef(x, y, z) = blocks[x][y][z];
            }
        }
    }
}

BlockType Chunk::getBlockType(int x, int y, int z) const {
    if(x < 0 || y < 0 || z < 0 || x >= 16 || y >= 256 || z >= 16) {
        return EMPTY;
    }
    return m_blocks[z * (256 * 16) + y * 16 + x];
}

BlockType* Chunk::getBlockTypeRef(int x, int y, int z) {
    return &(m_blocks[z * (256 * 16) + y * 16 + x]);
}

int64_t getKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    x = (x << 32) | 0x00000000ffffffff;
    z = z | 0xffffffff00000000;
    xz = z & x;
    return xz;
}

void Chunk::createIndicesOpaque(vector<GLuint> &idx) {
    for(int i = 0; i < facesOpaque; i++){
        idx.push_back(i*4);
        idx.push_back(i*4+1);
        idx.push_back(i*4+2);
        idx.push_back(i*4);
        idx.push_back(i*4+2);
        idx.push_back(i*4+3);
    }
}

void Chunk::createIndicesTransparent(vector<GLuint> &idx) {
    for(int i = 0; i < facesTransparent; i++){
        idx.push_back(i*4);
        idx.push_back(i*4+1);
        idx.push_back(i*4+2);
        idx.push_back(i*4);
        idx.push_back(i*4+2);
        idx.push_back(i*4+3);
    }
}

void Chunk::create() {
    vector<GLuint> c_idx;

    createIndicesOpaque(c_idx);

    count = c_idx.size();

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), &c_idx[0], GL_STATIC_DRAW);

    generatePosNorUVOpaque();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVOpaque);
    context->glBufferData(GL_ARRAY_BUFFER, c_vert_pos_nor_uv_opaque.size() * sizeof(glm::vec4), &c_vert_pos_nor_uv_opaque[0], GL_STATIC_DRAW);
}

void Chunk::createTransparent() {
    vector<GLuint> c_idx;
    createIndicesTransparent(c_idx);
    countT = c_idx.size();

    generateIdxT();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxT);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, countT * sizeof(GLuint), &c_idx[0], GL_STATIC_DRAW);

    generatePosNorUVTransparent();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVTransparent);
    context->glBufferData(GL_ARRAY_BUFFER, c_vert_pos_nor_uv_transparent.size() * sizeof(glm::vec4), &c_vert_pos_nor_uv_transparent[0], GL_STATIC_DRAW);
}
