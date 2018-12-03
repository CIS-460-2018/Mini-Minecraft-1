#pragma once
#include <QList>
#include <la.h>
#include <QOpenGLContext>

#include <scene/terrain.h>
#include "chunk.h"
#include "blocktype.h"
#include "lsystem.h"

using namespace glm;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.

class Chunk;
enum BlockType: unsigned char;

class Terrain
{
private:
    int64_t getKey(int x, int y, bool chunked) const;
    OpenGLContext* context;
public:
    QHash<int64_t, Chunk*> chunkMap;
    glm::ivec3 dimensions;
    int x_boundary_start, x_boundary_end, y_boundary_start, y_boundary_end, z_boundary_start, z_boundary_end;

    Terrain(OpenGLContext* c);

    Terrain(OpenGLContext* c, int x_boundary_end, int y_boundary_end, int z_boundary_end);

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
    void CreateTestScene();

    void updateScene();                                // Creates the VBOS for every chunk. Should only be called after all 16x256x16 blocks are assigned a blocktype
                                                           // and you are ready to set a VBO
    void createVertexPosNorCol(Chunk* c, int xChunk, int zChunk);

    bool checkEmpty(int x, int y, int z, Chunk* c, int xChunk, int zChunk);

    void drawRoute(Turtle startTurtle, Turtle nextTurtle);

};
