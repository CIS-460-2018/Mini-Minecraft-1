#ifndef PLAYER_H
#define PLAYER_H
#include "la.h"
#include "camera.h"
#include "scene/terrain.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSound>

class Player
{
private:
    glm::vec4 velocity = glm::vec4(0, 0, 0, 1); // player's velocity in 3D space, initially zero
    const float G = 9.81f; // gravitational constant
    glm::vec4 acceleration = glm::vec4(0, -G, 0, 1);
    Camera* camera; // pointer to the camera
    int key = 0; // key tracker
    QMouseEvent* mouse; // mouse tracker
    bool isFlyMode; // determine the fly/gravity mode
    bool inLiquid; // in lava or water
    bool grounded = false;

    QSound* footstep;
    QSound* underwater;
    QSound* splash;

    bool isFootStep;

public:
    Player(Camera* cam);

    glm::vec4 position = glm::vec4(20, 200, 20, 1); // player's position in 3D space, initially at (20, 160, 20)

    void updateKey(QKeyEvent* e); // updates the most recently pressed keyboard key

    void updateMouse(QMouseEvent* e); // updates the most recently clicked mouse key

    void updateVelocity(); // update the velocity of the player

    void checkCollision(float dt, Terrain *t);

    std::vector<glm::vec4> getPointsToCheck(glm::vec4 direction);

    bool isLiquidBlock(BlockType b);

    glm::vec4 getPosition();

    void resetKey();

    void stopFootstep();
};

#endif // PLAYER_H
