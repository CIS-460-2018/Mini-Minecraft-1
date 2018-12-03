#include "quadrangle.h"

void Quadrangle::create()
{
    GLuint idx[6];
    idx[0] = 0;
    idx[1] = 1;
    idx[2] = 2;
    idx[3] = 0;
    idx[4] = 2;
    idx[5] = 3;

    glm::vec4 col;
    if(inBlock == WATER) {
        col = glm::vec4(0, 0, 1, 0.3);
    } else if(inBlock == LAVA){
        col = glm::vec4(1, 0, 0, 0.3);
    } else {
        col = glm::vec4(0, 0, 0, 0);
    }
    vert_pos_nor_col[0] = glm::vec4(-1, -1, 0, 1);
    vert_pos_nor_col[1] = glm::vec4(0, 0, 1, 0);
    vert_pos_nor_col[2] = col;
    vert_pos_nor_col[3] = glm::vec4(1, -1, 0, 1);
    vert_pos_nor_col[4] = glm::vec4(0, 0, 1, 0);
    vert_pos_nor_col[5] = col;
    vert_pos_nor_col[6] = glm::vec4(1, 1, 0, 1);
    vert_pos_nor_col[7] = glm::vec4(0, 0, 1, 0);
    vert_pos_nor_col[8] = col;
    vert_pos_nor_col[9] = glm::vec4(-1, 1, 0, 1);
    vert_pos_nor_col[10] = glm::vec4(0, 0, 1, 0);
    vert_pos_nor_col[11] = col;

    count = 6;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(glm::vec4), vert_pos_nor_col, GL_STATIC_DRAW);
}


void Quadrangle::setInBlock(BlockType t) {
    inBlock = t;
}
