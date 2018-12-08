#include "worldaxes.h"

void WorldAxes::create()
{

    GLuint idx[6] = {0, 1, 2, 3, 4, 5};
    glm::vec4 pos[6] = {glm::vec4(32,128,32,1), glm::vec4(40,128,32,1),
                        glm::vec4(32,128,32,1), glm::vec4(32,136,32,1),
                        glm::vec4(32,128,32,1), glm::vec4(32,128,40,1)};
    glm::vec4 col[6] = {glm::vec4(1,0,0,1), glm::vec4(1,0,0,1),
                        glm::vec4(0,1,0,1), glm::vec4(0,1,0,1),
                        glm::vec4(0,0,1,1), glm::vec4(0,0,1,1)};
    glm::vec4 nor[6] = {glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0),
                        glm::vec4(0, 0, 1, 0), glm::vec4(0, 0, 1, 0)};
    glm::vec4 posNorCol[18];
    for(int i = 0; i < 6; i++) {
        posNorCol[i*3] = pos[i];
        posNorCol[i*3+1] = nor[i];
        posNorCol[i*3+2] = col[i];
    }

    count = 6;

    generateIdxPosNorCol();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxCol);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), idx, GL_STATIC_DRAW);
    generatePosNorCol();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    context->glBufferData(GL_ARRAY_BUFFER, count * 3 * sizeof(glm::vec4), posNorCol, GL_STATIC_DRAW);
}

GLenum WorldAxes::drawMode()
{
    return GL_LINES;
}
