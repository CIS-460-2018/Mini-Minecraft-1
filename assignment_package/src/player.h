#ifndef PLAYER_H
#define PLAYER_H
#include "la.h"
#include "camera.h"
#include "scene/terrain.h"
#include <QKeyEvent>
#include <QMouseEvent>

/*
 * QUESTIONS:
 * 1) "A set of variables to track the relevant inputs from the mouse and keyboard" should this be a set of pressed keys, or can i just
 * store the information on the most recently pressed key?
 * 2) Is it sufficient to press the F key once? Or do we need to hold down the F key?
 * 3) How do we make a player physics demo?
 * 4) velocity
 * 5) "Note that to handle the Player's "event listener" functions, you should simply invoke them from MyGL's various key and mouse
 * event functions, and pass them the relevant event. Unlike the base code setup, your MyGL should not directly modify the Player
 * or Camera from within its key or mouse event functions." more clarification on this part
 * 6) Timer update??
*/
class Player
{
private:
    glm::vec4 position = glm::vec4(5, 140, 5, 1); // player's position in 3D space, initially at (5, 130, 5)
    glm::vec4 velocity = glm::vec4(0, 0, 0, 1); // player's velocity in 3D space, initially zero
    const float G = 9.81f; // gravitational constant
    glm::vec4 acceleration = glm::vec4(0, -G, 0, 1);
    Camera* camera; // pointer to the camera
    int key = 0; // key tracker
    QMouseEvent* mouse; // mouse tracker
    bool isFlyMode; // determine the fly/gravity mode
    bool grounded = false;

    glm::vec2 m_mousePosPrev;
public:
    Player(Camera* cam);

    void updateKey(QKeyEvent* e); // updates the most recently pressed keyboard key

    void updateMouse(QMouseEvent* e); // updates the most recently clicked mouse key

    void move(); // move the player by the given amount

    void updateVelocity(int64_t dt); // update the velocity of the player

    void checkCollision(float dt, Terrain *t);

    glm::vec4 getPosition();

    void resetKey();
};

#endif // PLAYER_H
