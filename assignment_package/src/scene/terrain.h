#pragma once
#include <QList>
#include <la.h>
#include <QOpenGLContext>

#include <scene/terrain.h>
#include "chunk.h"
#include "blocktype.h"

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.

class Chunk;
enum BlockType: unsigned char;

class Terrain
{
private:
    int64_t getKey(int x, int y) const;
    OpenGLContext* context;
public:
    Terrain(OpenGLContext* c);

    void CreateTestScene();

    QHash<int64_t, Chunk*> chunkMap;

    glm::ivec3 dimensions;

    BlockType getBlockAt(int x, int y, int z) const;   // Given a world-space coordinate (which may have negative
                                                           // values) return the block stored at that point in space.
    void setBlockAt(int x, int y, int z, BlockType t); // Given a world-space coordinate (which may have negative
                                                           // values) set the block at that point in space to the
                                                           // given type.
};
