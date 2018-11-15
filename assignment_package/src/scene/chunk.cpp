#include "chunk.h"

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

void Chunk::createVertexPosNorCol(vector<glm::vec4> &posNorCol) {
    faces = 0;
    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 256; y++) {
            for(int z = 0; z < 16; z++) {
                BlockType t;
                if((t = getBlockType(x, y, z)) != EMPTY) {
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
                    if(getBlockType(x, y+1, z) == EMPTY) {
                        glm::vec4 normal = glm::vec4(0, 1, 0, 0);
                        posNorCol.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                    // bottom
                    if(getBlockType(x, y-1, z) == EMPTY) {
                        glm::vec4 normal = glm::vec4(0, -1, 0, 0);
                        posNorCol.push_back(glm::vec4(x, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                    //right
                    if(getBlockType(x+1, y, z) == EMPTY) {
                        glm::vec4 normal = glm::vec4(1, 0, 0, 0);
                        posNorCol.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                    //left
                    if(getBlockType(x-1, y, z) == EMPTY) {
                        glm::vec4 normal = glm::vec4(-1, 0, 0, 0);
                        posNorCol.push_back(glm::vec4(x, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                    //front
                    if(getBlockType(x, y, z+1) == EMPTY) {
                        glm::vec4 normal = glm::vec4(0, 0, 1, 0);
                        posNorCol.push_back(glm::vec4(x, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y+1, z+1, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                    //back
                    if(getBlockType(x, y, z-1) == EMPTY) {
                        glm::vec4 normal = glm::vec4(0, 0, -1, 0);
                        posNorCol.push_back(glm::vec4(x, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x+1, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        posNorCol.push_back(glm::vec4(x, y+1, z, 1.0f));
                        posNorCol.push_back(normal);
                        posNorCol.push_back(col);
                        faces++;
                    }
                }
            }
        }
    }
}

void Chunk::createIndices(vector<GLuint> &idx) {
    for(int i = 0; i < faces; i++){
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
    vector<glm::vec4> c_vert_pos_nor_col;

    createVertexPosNorCol(c_vert_pos_nor_col);
    createIndices(c_idx);

    count = c_idx.size();

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), &c_idx[0], GL_STATIC_DRAW);

    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, c_vert_pos_nor_col.size() * sizeof(glm::vec4), &c_vert_pos_nor_col[0], GL_STATIC_DRAW);
}
