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
    glm::vec2 diff = 0.2f * (pos - m_mousePosPrev);
    m_mousePosPrev = pos;
    camera->RotateAboutUp(-diff.x);
    camera->RotateAboutRight(-diff.y);
    camera->RecomputeAttributes();
}

void Player::updateVelocity(int64_t dt)
{
    if (key != 0) {
        float amount = 2.0f;
        if (key == Qt::Key_Space) {
            velocity = glm::vec4(0, 10, 0, 1); // jump with speed of 10
            acceleration = glm::vec4(0, -G, 0, 1);
            grounded = false;
        } else if (key == Qt::Key_W) {
            velocity = glm::vec4(0, 0, amount, 1);
            //camera->TranslateAlongLook(amount);
        } else if (key == Qt::Key_S) {
            velocity = glm::vec4(0, 0, -amount, 1);
            //camera->TranslateAlongLook(-amount);
        } else if (key == Qt::Key_D) {
            velocity = glm::vec4(amount, 0, 0, 1);
            //camera->TranslateAlongRight(amount);
        } else if (key == Qt::Key_A) {
            velocity = glm::vec4(-amount, 0, 0, 1);
            //camera->TranslateAlongRight(-amount);
        } else if (key == Qt::Key_F) {
            if (isFlyMode) {
                isFlyMode = false;
                acceleration = glm::vec4(0, -G, 0, 1);
                camera->getInitialHeightLevel();
            } else {
                isFlyMode = true;
                acceleration = glm::vec4(0, 0, 0, 1);
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

void Player::checkCollision(float dt, Terrain* t)
{
    velocity += acceleration * (dt / 1000.0f);
    glm::vec4 incrVec = velocity * (dt / 10000.0f);
    glm::vec4 potPos = position; // potential final position
    int counter = 0;
    while (counter < 10) {
        // check for 8 corners
        BlockType one = t->getBlockAt(position.x + 1, position.y + 2, position.z + 1);
        BlockType two = t->getBlockAt(position.x + 1, position.y + 2, position.z - 1);
        BlockType three = t->getBlockAt(position.x - 1, position.y + 2, position.z + 1);
        BlockType four = t->getBlockAt(position.x - 1, position.y + 2, position.z - 1);
        BlockType five = t->getBlockAt(position.x + 1, position.y - 2, position.z + 1);
        BlockType six = t->getBlockAt(position.x + 1, position.y - 2, position.z - 1);
        BlockType seven = t->getBlockAt(position.x - 1, position.y - 2, position.z + 1);
        BlockType eight = t->getBlockAt(position.x - 1, position.y - 2, position.z - 1);
        if (one != 0 || two != 0 || three != 0 || four != 0 || five != 0 || six != 0 || seven != 0 || eight != 0) {
            if (!grounded) {
                position += glm::vec4(0, 1, 0, 1);
                acceleration = glm::vec4(0, 0, 0, 1);
                grounded = true;
            }
            break;
        }
        counter++;
        potPos += incrVec;
        if (incrVec.x != 0.0f) {
            camera->TranslateAlongRight(incrVec.x);
        }
        if (incrVec.y != 0.0f) {
            camera->TranslateAlongUp(incrVec.y);
        }
        if (incrVec.z != 0.0f) {
            camera->TranslateAlongLook(incrVec.z);
        }
        position = potPos;
    }
    velocity = glm::vec4(0, 0, 0, 1); // reset velocity
}

glm::vec4 Player::getPosition()
{
    return position;
}

void Player::resetKey()
{
    key = 0;
}
