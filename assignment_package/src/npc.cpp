#include "npc.h"

NPC::NPC(Terrain* t, OpenGLContext *context) : Drawable(context), terrain(t)
{}

void NPC::updateVelocity() {}

void NPC::move() {

}

void NPC::checkCollision(float dt) {

}

std::vector<glm::vec4> NPC::getPointsToCheck(glm::vec4 direction) {
}

bool NPC::isLiquidBlock(BlockType b) {
    return false;
}

glm::vec4 NPC::getPosition() {
    return position;
}

void NPC::generatePosition() {}

void NPC::create() {

    vector<GLuint> indices; // store the indices
    for(int i = 0; i < 36; i++){
        indices.push_back(i*4);
        indices.push_back(i*4+1);
        indices.push_back(i*4+2);
        indices.push_back(i*4);
        indices.push_back(i*4+2);
        indices.push_back(i*4+3);
    }
    count = indices.size();

    // form the interleaved VBO

    /*generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), &c_idx[0], GL_STATIC_DRAW);

    generatePosNorUVOpaque();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVOpaque);
    context->glBufferData(GL_ARRAY_BUFFER, c_vert_pos_nor_uv_opaque.size() * sizeof(glm::vec4), &c_vert_pos_nor_uv_opaque[0], GL_STATIC_DRAW);*/
}
