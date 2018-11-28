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
}

void Player::updateKey(QKeyEvent *e)
{
    key = e->key();
}

void Player::updateMouse(QMouseEvent *e)
{
    mouse = e;
    glm::vec2 pos(e->pos().x(), e->pos().y());
    glm::vec2 diff = 0.1f * (pos - m_mousePosPrev);
    m_mousePosPrev = pos;
    camera->RotateAboutUp(-diff.x);
    camera->RotateAboutRight(-diff.y);
    camera->RecomputeAttributes();
}

void Player::updateVelocity()
{
    if (key != 0) {
        float amount = 2.0f;
        if (key == Qt::Key_Space) {
            velocity += glm::vec4(0, 7, 0, 1); // jump with speed of 10
            acceleration = glm::vec4(0, -G, 0, 1);
            grounded = false;
        } else if (key == Qt::Key_W) {
            velocity += glm::vec4(0, 0, amount, 1);
        } else if (key == Qt::Key_S) {
            velocity += glm::vec4(0, 0, -amount, 1);
        } else if (key == Qt::Key_D) {
            velocity += glm::vec4(amount, 0, 0, 1);
        } else if (key == Qt::Key_A) {
            velocity += glm::vec4(-amount, 0, 0, 1);
        } else if (key == Qt::Key_F) {
            if (isFlyMode) {
                isFlyMode = false;
                camera->getInitialHeightLevel();
            } else {
                isFlyMode = true;
                camera->TranslateAlongUp(5.0f);
            }
        } else if (key == Qt::Key_E) {
            if (isFlyMode) {
                camera->TranslateAlongUp(amount / 10.0f);
            }
        } else if (key == Qt::Key_Q) {
            if (isFlyMode) {
                camera->TranslateAlongUp(-amount / 10.0f);
            }
        }
    }
}

void Player::checkCollision(float dt, Terrain* t)
{
    velocity += glm::vec4(acceleration.x * dt, acceleration.y * dt, acceleration.z * dt, 1); // update the velocity with acceleration
    glm::vec4 posIncrease = velocity * (dt);
    posIncrease = glm::vec4(posIncrease.x, posIncrease.y, posIncrease.z, 1);
    glm::vec4 dir = glm::normalize(posIncrease);
    float maxDist = glm::length(posIncrease);
    float curDist = 0;
    if (isFlyMode) {
        if (posIncrease.z != 0) {
            position += posIncrease;
            camera->TranslateAlongLook(posIncrease.z);
            velocity = glm::vec4(0, 0, 0, 1);
        }
        if (posIncrease.x != 0) {
            position += posIncrease;
            camera->TranslateAlongRight(posIncrease.x);
            velocity = glm::vec4(0, 0, 0, 1);
        }
        if (posIncrease.y != 0) {
            position += posIncrease;
            camera->TranslateAlongUp(posIncrease.y);
            velocity = glm::vec4(0, 0, 0, 1);
        }
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
            if (currBlock != EMPTY) {
                std::cout << position.y << std::endl;
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
                if (b1 == EMPTY && b2 == EMPTY && b3 == EMPTY && b4 == EMPTY) {
                    grounded = false;
                    acceleration = glm::vec4(0, -G, 0, 1);
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
