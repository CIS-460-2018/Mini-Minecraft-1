#pragma once
#include <QList>
#include <la.h>

using namespace glm;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE
};

class Terrain
{
public:
    Terrain();
    Terrain(int x_boundary_end, int y_boundary_end, int z_boundary_end);
    BlockType m_blocks[64][256][64];                    // A 3D list of the blocks in the world.
                                                           // You'll need to replace this with a far more
                                                           // efficient system of storing terrain.
    void CreateTestScene();

    int x_boundary_start, x_boundary_end, y_boundary_start, y_boundary_end, z_boundary_start, z_boundary_end;

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
//    vec2 randomfunc(vec2 p);

//    float interpNoise2D(float x, float y);

//    float fbm(float x, float y);
};
