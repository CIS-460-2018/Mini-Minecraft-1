#include "quadrangle.h"

void Quadrangle::create()
{
    /*
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
    generateIdxPosNorCol();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxCol);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of sfunction calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePosNorCol(); // TODO: figure out which (opaque or transparent) to add to
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(glm::vec4), vert_pos_nor_col, GL_STATIC_DRAW);
    */
    GLuint idx[6]{0, 1, 2, 0, 2, 3};
    glm::vec4 vert_pos[4] {glm::vec4(-1.f, -1.f, 0.99f, 1.f),
                           glm::vec4(1.f, -1.f, 0.99f, 1.f),
                           glm::vec4(1.f, 1.f, 0.99f, 1.f),
                           glm::vec4(-1.f, 1.f, 0.99f, 1.f)};

    glm::vec2 vert_UV[4] {glm::vec2(0.f, 0.f),
                          glm::vec2(1.f, 0.f),
                          glm::vec2(1.f, 1.f),
                          glm::vec2(0.f, 1.f)};

    count = 6;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdx();
    // Tell OpenGL that we want to perform subsequent operations on the VBO referred to by bufIdx
    // and that it will be treated as an element array buffer (since it will contain triangle indices)
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    // Pass the data stored in cyl_idx into the bound buffer, reading a number of bytes equal to
    // CYL_IDX_COUNT multiplied by the size of a GLuint. This data is sent to the GPU to be read by shader programs.
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of function calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePos();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec4), vert_pos, GL_STATIC_DRAW);
    generateUV();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    context->glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec2), vert_UV, GL_STATIC_DRAW);
}


void Quadrangle::setInBlock(BlockType t) {
    inBlock = t;
}
