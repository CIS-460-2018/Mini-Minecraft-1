#ifndef NPC_H
#define NPC_H
#include "la.h"
#include "scene/terrain.h"
#include "drawable.h"

class NPC : public Drawable
{
private:
    glm::vec4 position = glm::vec4(20, 200, 20, 1);
    glm::vec4 velocity = glm::vec4(0, 0, 0, 1); // npc's velocity in 3D space, initially zero
    const float G = 9.81f; // gravitational constant
    glm::vec4 acceleration = glm::vec4(0, -G, 0, 1);
    bool inLiquid = false; // in lava or water
    bool grounded = false;
    Terrain* terrain;
    vector<glm::vec4> vbo_vert_pos_nor_uv_transparent;
    bool didCollide = false;
    glm::vec4 lastDirection;
public:
    NPC(Terrain* t, OpenGLContext* context);

    void updateVelocity(); // update the velocity of the NPC

    void move();

    void checkCollision(float dt);

    std::vector<glm::vec4> getPointsToCheck(glm::vec4 direction);

    bool isLiquidBlock(BlockType b);

    glm::vec4 getPosition();

    void generatePosition();

    void create() override;
};

#endif // NPC_H
