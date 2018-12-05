#include "cursor.h"
#include <iostream>

void Cursor::create()
{

    GLuint idx[8];
    for (int i = 0; i < 8; i++) {
        idx[i] = i;
    }

    glm::vec4 pos[8] = {center + right, center + (right * 5.0f),
                        center + up, center + (up * 5.0f),
                        center - right, center - (right * 5.0f),
                        center - up, center - (up * 5.0f)};
    glm::vec4 col[8] = {glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1),
                        glm::vec4(1,1,1,1), glm::vec4(1,1,1,1)};
    glm::vec4 nor[8] = {glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0)};
    glm::vec4 posNorCol[24];
    for(int i = 0; i < 8; i++) {
        posNorCol[i*3] = pos[i];
        posNorCol[i*3+1] = nor[i];
        posNorCol[i*3+2] = col[i];
    }

    count = 8;

    // Create a VBO on our GPU and store its handle in bufIdx
    generateIdxPosNorCol();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxCol);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), idx, GL_STATIC_DRAW);

    // The next few sets of sfunction calls are basically the same as above, except bufPos and bufNor are
    // array buffers rather than element array buffers, as they store vertex attributes like position.
    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(glm::vec4), posNorCol, GL_STATIC_DRAW);
}

GLenum Cursor::drawMode()
{
    return GL_LINES;
}

void Cursor::setCenter(glm::vec4 look)
{
    center = look;
    //center.z += 10.0f;
}

void Cursor::setRight(glm::vec4 right)
{
    this->right = right;
}

void Cursor::setUp(glm::vec4 up)
{
    this->up = up;
}
