#include "npc.h"
#include <iostream>

int NPC::posGenerationConstX = 10;
int NPC::posGenerationConstZ = 10;

NPC::NPC(Terrain* t, OpenGLContext *context) : Drawable(context), terrain(t)
{}

void NPC::updateVelocity() {
    // decide the new velocity direction randomly if a collision happens
    if (didCollide) {
        float amount = 1.0f;
        if(inLiquid) {
            amount = amount * 2 / 3;
        }
        if (lastDirection.y != 0) {
            if (position.x > position.z) {
                velocity = glm::vec4(0, 0, amount, 1);
            } else {
                velocity = glm::vec4(amount, 0, 0, 1);
            }
        }
        if (lastDirection.x > 0) {
            if (grounded) {
                velocity = glm::vec4(amount, 7, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -G, 0, 1);
            }
            if(inLiquid) {
                velocity = glm::vec4(amount, 2.5, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
            }
            grounded = false;

        } else if (lastDirection.x < 0) {
            if (grounded) {
                velocity = glm::vec4(-amount, 7, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -G, 0, 1);
            }
            if(inLiquid) {
                velocity = glm::vec4(-amount, 2.5, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
            }
            grounded = false;

        }
        if (lastDirection.z > 0) {
            if (grounded) {
                velocity = glm::vec4(0, 7, amount / 2.0f, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -G, 0, 1);
            }
            if(inLiquid) {
                velocity = glm::vec4(0, 2.5, amount / 2.0f, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
            }
            grounded = false;
        } else if (lastDirection.z < 0) {
            if (grounded) {
                velocity = glm::vec4(0, 7, -amount / 2.0f, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -G, 0, 1);
            }
            if(inLiquid) {
                velocity = glm::vec4(0, 2.5, -amount / 2.0f, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
            }
            grounded = false;
        }
        didCollide = false;
    }
}

void NPC::checkCollision(float dt) {

    BlockType inBlock = terrain->getBlockAt(position.x, position.y, position.z);
    if(isLiquidBlock(inBlock)) {
        if(inLiquid == false) {
            acceleration = glm::vec4(acceleration.x * 2 / 3, acceleration.y * 2 / 3, acceleration.z * 2 / 3, 1);
            velocity = glm::vec4(velocity.x * 2 / 3, velocity.y * 2 / 3, velocity.z * 2 / 3, 1);
            inLiquid = true;
        }
    } else {
        if(inLiquid == true) {
            acceleration = glm::vec4(acceleration.x * 3 / 2, acceleration.y * 3 / 2, acceleration.z * 3 / 2, 1);
            velocity = glm::vec4(velocity.x * 3 / 2, velocity.y * 3 / 2, velocity.z * 3 / 2, 1);
            inLiquid = false;
        }
    }
    velocity += glm::vec4(acceleration.x * dt, acceleration.y * dt, acceleration.z * dt, 1); // update the velocity with acceleration
    glm::vec4 posIncrease = velocity * (dt);
    posIncrease = glm::vec4(posIncrease.x, posIncrease.y, posIncrease.z, 1);
    glm::vec4 dir = glm::normalize(posIncrease);
    float maxDist = glm::length(posIncrease);
    float curDist = 0;
    bool isColFound = false;
    while (curDist < maxDist && !isColFound) {
        // check if incrementing position results in collision
        curDist += 0.10f;
        std::vector<glm::vec4> toCheck = getPointsToCheck(dir);
        for (glm::vec4 point : toCheck) {
            glm::vec4 potentialPos = point + curDist * dir;
            BlockType currBlock = terrain->getBlockAt(potentialPos.x, potentialPos.y, potentialPos.z);
            if (currBlock != EMPTY && !isLiquidBlock(currBlock)) {
                didCollide = true;
                if (!grounded) {
                    //std::cout << "Sheep X: " << position.x << "Y: " << position.y << "Z: " << position.z << std::endl;
                    velocity = glm::vec4(0, 0, 0, 1);
                    if (dir.y > 0) {
                        // head bump check
                    } else {
                        grounded = true;
                        acceleration = glm::vec4(0, 0, 0, 1);
                    }
                }
                lastDirection = dir;
                //std::cout << "now the last direction is x:" << lastDirection.x << " y:" << lastDirection.y << " z:" << lastDirection.z << std::endl;
                curDist -= 0.10f;
                isColFound = true;
                break;
            } else {
                // check if NPC is falling off from a cliff
                float yIncr = position.y - 0.5f;
                glm::vec4 corner1 = glm::vec4(position.x - 1.5f, yIncr, position.z - 0.75f, 0);
                glm::vec4 corner2 = glm::vec4(position.x - 1.5f, yIncr, position.z + 0.75f, 0);
                glm::vec4 corner3 = glm::vec4(position.x + 1.0f, yIncr, position.z - 0.75f, 0);
                glm::vec4 corner4 = glm::vec4(position.x + 1.0f, yIncr, position.z + 0.75f, 0);
                BlockType b1 = terrain->getBlockAt(corner1.x, corner1.y, corner1.z);
                BlockType b2 = terrain->getBlockAt(corner2.x, corner2.y, corner2.z);
                BlockType b3 = terrain->getBlockAt(corner3.x, corner3.y, corner3.z);
                BlockType b4 = terrain->getBlockAt(corner4.x, corner4.y, corner4.z);
                if ((b1 == EMPTY || isLiquidBlock(b1)) && (b2 == EMPTY || isLiquidBlock(b2)) &&
                    (b3 == EMPTY || isLiquidBlock(b3)) && (b4 == EMPTY || isLiquidBlock(b4))) {
                    grounded = false;
                    if(b1 == EMPTY || b2 == EMPTY || b3 == EMPTY || b4 == EMPTY) {
                        acceleration = glm::vec4(0, -G, 0, 1);
                    } else {
                        acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
                    }
                }
            }
        }
    }
    // update the position accordingly
    glm::vec4 incrPos = curDist * dir;
    position += incrPos;
}

std::vector<glm::vec4> NPC::getPointsToCheck(glm::vec4 direction) {
    std::vector<glm::vec4> points; // store the points in here
    if (direction.x > 0) {
        float xIncr = position.x + 1.0f;
        points.push_back(glm::vec4(xIncr, position.y, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z, 0));
    } else if (direction.x < 0) {
        float xIncr = position.x - 1.5f;
        points.push_back(glm::vec4(xIncr, position.y, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z + 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z - 0.75f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z, 0));
    }
    if (direction.y > 0) {
        float yIncr = position.y + 1.0f;
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z - 0.75f, 0));
    } else if (direction.y < 0) {
        float yIncr = position.y - 1.0f;
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x + 1.0f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.5f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z - 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x - 1.0f, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z + 0.75f, 0));
        points.push_back(glm::vec4(position.x, yIncr, position.z - 0.75f, 0));
    }
    if (direction.z > 0) {
        float zIncr = position.z + 0.75f;
        points.push_back(glm::vec4(position.x + 1.0f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x + 1.0f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x + 1.0f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y - 1.0f, zIncr, 0));
    } else if (direction.z < 0) {
        float zIncr = position.z - 0.75f;
        points.push_back(glm::vec4(position.x + 1.0f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x + 1.0f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x + 1.0f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.0f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 1.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x, position.y - 1.0f, zIncr, 0));
    }
    return points;
}

bool NPC::isLiquidBlock(BlockType b) {
    return false;
}

glm::vec4 NPC::getPosition() {
    return position;
}

void NPC::generatePosition() {
    position = glm::vec4(posGenerationConstX, 400, posGenerationConstZ, 1);
}

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
    glm::vec2 tex = glm::vec2(1.0f / 16.0f * 5, 1.0f / 16.0f * 13);
    float cos = 2.0f;

    vbo_vert_pos_nor_uv_transparent.clear();

    /// add the body vertices
    // add front
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add back
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add left
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add right
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add top
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y + 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add bottom
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;


    /// add head vertices
    // add front
    glm::vec2 faceTexture = glm::vec2(57, 197) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(faceTexture.x, faceTexture.y, cos, 0));
    faceTexture = glm::vec2(57, 146) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(faceTexture.x, faceTexture.y, cos, 0));
    faceTexture = glm::vec2(108, 146) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(faceTexture.x, faceTexture.y, cos, 0));
    faceTexture = glm::vec2(108, 197) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(faceTexture.x, faceTexture.y, cos, 0));
    // add back
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add left
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add right
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add top
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 1.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;
    // add bottom
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y -= 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 1.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.y += 1.0f / 16.0f;
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.5f, position.y + 0.25f, position.z + 0.5f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(tex.x, tex.y, cos, 0));
    tex.x -= 1.0f / 16.0f;

    /// add back left leg vertices
    vec2 legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    // add front
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add back
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add left
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add right
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add top
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add bottom
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));

    /// add back right leg vertices
    // add front
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add back
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add right
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add left
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 0.1f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 0.1f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add top
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add bottom
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z - 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));

    /// add front left leg vertices
    // add front
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add back
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add left
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add right
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add top
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add bottom
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x - 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));

    /// add front right leg vertices
    // add front
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, -1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add back
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 0, 1, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add right
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add left
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(-1, 0, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add top
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 0.5f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, 1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    // add bottom
    legTexture = glm::vec2(30, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(30, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.75f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 11) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 1.0f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));
    legTexture = glm::vec2(58, 39) * (1 / 256.0f);
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(position.x + 0.35f, position.y - 1.0f, position.z + 0.6f, 1));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(0, -1, 0, 0));
    vbo_vert_pos_nor_uv_transparent.push_back(glm::vec4(legTexture.x, legTexture.y, cos, 0));

    generateIdx();
    context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    generatePosNorUVOpaque();
    context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVOpaque);
    context->glBufferData(GL_ARRAY_BUFFER, vbo_vert_pos_nor_uv_transparent.size() * sizeof(glm::vec4), &vbo_vert_pos_nor_uv_transparent[0], GL_STATIC_DRAW);
}
