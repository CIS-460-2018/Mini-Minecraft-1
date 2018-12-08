#ifndef NPC_H
#define NPC_H
#include "la.h"

class NPC
{
private:
    glm::vec4 position;
    glm::vec4 velocity = glm::vec4(0, 0, 0, 1); // npc's velocity in 3D space, initially zero
    const float G = 9.81f; // gravitational constant
    glm::vec4 acceleration = glm::vec4(0, -G, 0, 1);
    bool inLiquid; // in lava or water
    bool grounded = false;
public:
    NPC();

    void updateVelocity(); // update the velocity of the player

    //void checkCollision(float dt, Terrain *t);

    std::vector<glm::vec4> getPointsToCheck(glm::vec4 direction);

    //bool isLiquidBlock(BlockType b);

    //glm::vec4 getPosition();
};

#endif // NPC_H
