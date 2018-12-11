#pragma once
#include <QList>
#include <la.h>
#include <QOpenGLContext>

#include "chunk.h"
#include "blocktype.h"
#include "lsystem.h"
#include "biometype.h"

using namespace glm;
using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.

class Chunk;
enum BlockType: unsigned char;

class Terrain
{
private:
    OpenGLContext* context;
    LSystem *l_system_delta;
    LSystem *l_system_linear;
public:
    QHash<int64_t, Chunk*> chunkMap;
    QHash<int64_t, pair<glm::vec2, BiomeType>> biomeMap;

    bool hasChunk(int x, int z);
    void createNewChunk(glm::vec3 position);
    int64_t getKey(int x, int y, bool chunked) const;

    glm::ivec3 dimensions;
    int x_boundary_start, x_boundary_end, y_boundary_start, y_boundary_end, z_boundary_start, z_boundary_end;

    Terrain(OpenGLContext* c);

    Terrain(OpenGLContext* c, int x_boundary_end, int y_boundary_end, int z_boundary_end);

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
    BiomeType randBiome();
    void initializeBiomeMap();
    BlockType findTopBlock(int x, int z);
    void CreateTestScene();
    void updatePictureArea(int playerX, int playerZ, vector<vector<float>> newHeight);
    void updateColorPictureArea(int playerX, int playerZ, vector<vector<pair<float, BlockType>>> newBlocks);

    void updateChunk(glm::vec3 position);

    bool checkEmpty(int x, int y, int z, Chunk* c, int xChunk, int zChunk);
    void updateScene(); // creates the VBOS for every chunk. Should only be called after all 16x256x16 blocks are assigned a blocktype
                        // and you are ready to set a VBO
    void createVertexPosNorUV(Chunk* c, int xChunk, int zChunk);

    glm::vec2 getTexture(int faceNum); // return the corresponding uv coordinate for the top left face corner
//    vec2 randomfunc(vec2 p);

    void drawRoute(Turtle startTurtle, Turtle nextTurtle);
    void drawLSystem(LSystem *l_system);
    void drawBuilding(int x_start, int x_end, int z_start, int z_end, int gradient_start, int gradient_delta);
    void drawSpiralBuilding(int x_start, int x_end, int z_start, int z_end, int gradient_start, int gradient_delta);
    void drawDisc(int x, int z, int radius, int depth);
    void generateRandomWalkTunnel(int x, int z, int radius, int depth, int randomness_seed);
    void drawTunnelSegment(int start_x, int end_x, int start_z, int end_z, int depth);
    void drawLavaPool(int x, int z, int radius, int depth);


    // Multithreading
    QList<pair<pair<int, int>, Chunk*>> chunksToAdd;
    void addChunks();
};
