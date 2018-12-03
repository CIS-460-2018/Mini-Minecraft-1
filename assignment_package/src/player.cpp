#include "player.h"
#include "scene/terrain.h"
#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <algorithm>

Player::Player(Camera *cam)
{
    camera = cam;
    inLiquid = false;
}

void Player::updateKey(QKeyEvent *e)
{
    key = e->key();
}

void Player::updateMouse(QMouseEvent *e)
{
    mouse = e;
    glm::vec2 pos(e->pos().x(), e->pos().y());
    glm::vec2 diff = 0.4f * (pos - m_mousePosPrev);
    m_mousePosPrev = pos;
    camera->RotateAboutUp(-diff.x);
    camera->RotateAboutRight(-diff.y);
    camera->RecomputeAttributes();
}

void Player::updateVelocity()
{
    if (key != 0) {
        float amount = 2.0f;
        if(inLiquid) {
            amount = amount * 2 / 3;
        }
        if (key == Qt::Key_Space) {
            // allow the player to jump only if standing on the ground
            if (grounded) {
                velocity = glm::vec4(0, 7, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -G, 0, 1);
            }
            if(inLiquid) {
                velocity = glm::vec4(0, 2.5, 0, 1); // jump with speed of 7
                acceleration = glm::vec4(0, -2.f / 3.f * G, 0, 1);
            }

            grounded = false;
        } else if (key == Qt::Key_W) {
            if (isFlyMode) {
                camera->TranslateAlongLook(amount);
            } else {
                glm::vec3 v = amount * camera->look;
                v.y = 0;
                velocity += glm::vec4(v, 1);
            }
        } else if (key == Qt::Key_S) {
            if (isFlyMode) {
                camera->TranslateAlongLook(-amount);
            } else {
                glm::vec3 v = -amount * camera->look;
                v.y = 0;
                velocity += glm::vec4(v, 1);
            }
        } else if (key == Qt::Key_D) {
            if (isFlyMode) {
                camera->TranslateAlongRight(amount);
            } else {
                glm::vec3 v = amount * camera->right;
                v.y = 0;
                velocity += glm::vec4(v, 1);
            }
        } else if (key == Qt::Key_A) {
            if (isFlyMode) {
                camera->TranslateAlongRight(-amount);
            } else {
                glm::vec3 v = -amount * camera->right;
                v.y = 0;
                velocity += glm::vec4(v, 1);
            }
        } else if (key == Qt::Key_F) {
            if (isFlyMode) {
                isFlyMode = false;
                acceleration = glm::vec4(0, -G, 0, 1);
                if(inLiquid) {
                    acceleration = glm::vec4(0, -2 / 3 * G, 0, 1);
                }
                grounded = false;
            } else {
                isFlyMode = true;
                position += 5.0f;
                camera->TranslateAlongUp(5.0f);
            }
        } else if (key == Qt::Key_E) {
            if (isFlyMode) {
                camera->TranslateAlongUp(amount);
            }
        } else if (key == Qt::Key_Q) {
            if (isFlyMode) {
                camera->TranslateAlongUp(-amount);
            }
        }
    }
}

bool Player::isLiquidBlock(BlockType b) {
    return (b == WATER || b == LAVA);
}

void Player::checkCollision(float dt, Terrain* t)
{
    BlockType inBlock = t->getBlockAt(position.x, position.y, position.z);
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
    if (isFlyMode) {
        position = glm::vec4(camera->eye, 1);
        velocity = glm::vec4(0, 0, 0, 1);
        return; // player is not subject to terrain collisions during fly mode
    }
    bool isColFound = false;
    while (curDist < maxDist && !isColFound) {
        // check if incrementing position causes in collision
        curDist += 0.25f;
        std::vector<glm::vec4> toCheck = getPointsToCheck(dir);
        for (glm::vec4 point : toCheck) {
            glm::vec4 potentialPos = point + curDist * dir;
            BlockType currBlock = t->getBlockAt(potentialPos.x, potentialPos.y, potentialPos.z);
            if (currBlock != EMPTY && !isLiquidBlock(currBlock)) {
                if (!grounded) {
                    grounded = true;
                    velocity = glm::vec4(0, 0, 0, 1);
                    acceleration = glm::vec4(0, 0, 0, 1);
                }
                curDist -= 0.25f;
                isColFound = true;
                break;
            } else {
                // check if player is falling off from a cliff
                float yIncr = position.y - 0.5f;
                glm::vec4 corner1 = glm::vec4(position.x + 0.5f, yIncr, position.z - 0.5f, 0);
                glm::vec4 corner2 = glm::vec4(position.x + 0.5f, yIncr, position.z + 0.5f, 0);
                glm::vec4 corner3 = glm::vec4(position.x - 0.5f, yIncr, position.z - 0.5f, 0);
                glm::vec4 corner4 = glm::vec4(position.x - 0.5f, yIncr, position.z + 0.5f, 0);
                BlockType b1 = t->getBlockAt(corner1.x, corner1.y, corner1.z);
                BlockType b2 = t->getBlockAt(corner2.x, corner2.y, corner2.z);
                BlockType b3 = t->getBlockAt(corner3.x, corner3.y, corner3.z);
                BlockType b4 = t->getBlockAt(corner4.x, corner4.y, corner4.z);
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
    camera->eye = glm::vec3(position.x, position.y, position.z);
    camera->ref += glm::vec3(incrPos.x, incrPos.y, incrPos.z);
    if (grounded) {
        velocity = glm::vec4(0, 0, 0, 1);
    }
}

std::vector<glm::vec4> Player::getPointsToCheck(glm::vec4 direction) {
    std::vector<glm::vec4> points; // store the points in here
    if (direction.x > 0) {
        float xIncr = position.x + 0.5f;
        points.push_back(glm::vec4(xIncr, position.y, position.z - 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z - 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z - 0.5f, 0));
    } else if (direction.x < 0) {
        float xIncr = position.x - 0.5f;
        points.push_back(glm::vec4(xIncr, position.y, position.z - 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y + 1.0f, position.z - 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z + 0.5f, 0));
        points.push_back(glm::vec4(xIncr, position.y - 1.0f, position.z - 0.5f, 0));
    }
    if (direction.y > 0) {
        float yIncr = position.y + 1.0f;
        points.push_back(glm::vec4(position.x + 0.5f, yIncr, position.z - 0.5f, 0));
        points.push_back(glm::vec4(position.x + 0.5f, yIncr, position.z + 0.5f, 0));
        points.push_back(glm::vec4(position.x - 0.5f, yIncr, position.z - 0.5f, 0));
        points.push_back(glm::vec4(position.x - 0.5f, yIncr, position.z + 0.5f, 0));
    } else if (direction.y < 0) {
        float yIncr = position.y - 1.0f;
        points.push_back(glm::vec4(position.x + 0.5f, yIncr, position.z - 0.5f, 0));
        points.push_back(glm::vec4(position.x + 0.5f, yIncr, position.z + 0.5f, 0));
        points.push_back(glm::vec4(position.x - 0.5f, yIncr, position.z - 0.5f, 0));
        points.push_back(glm::vec4(position.x - 0.5f, yIncr, position.z + 0.5f, 0));
    }
    if (direction.z > 0) {
        float zIncr = position.z + 0.5f;
        points.push_back(glm::vec4(position.x + 0.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x + 0.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x + 0.5f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y - 1.0f, zIncr, 0));
    } else if (direction.z < 0) {
        float zIncr = position.z - 0.5f;
        points.push_back(glm::vec4(position.x + 0.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y, zIncr, 0));
        points.push_back(glm::vec4(position.x + 0.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y + 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x + 0.5f, position.y - 1.0f, zIncr, 0));
        points.push_back(glm::vec4(position.x - 0.5f, position.y - 1.0f, zIncr, 0));
    }
    return points;
}

glm::vec4 Player::getPosition()
{
    return position;
}

void Player::resetKey()
{
    key = 0;
}
