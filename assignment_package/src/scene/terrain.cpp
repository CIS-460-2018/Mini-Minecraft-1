#include <scene/terrain.h>

#include <scene/cube.h>

Terrain::Terrain(OpenGLContext* c) : dimensions(64, 256, 64), context(c)
{}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    return chunkMap[getKey(x, z)]->getBlockType(x%16, y, z%16);
}

int64_t Terrain::getKey(int x, int z) const {
    int64_t xz = 0xffffffffffffffff;
    int64_t chunkx = x/16;
    int64_t chunkz = z/16;
    chunkx = (chunkx << 32) | 0x00000000ffffffff;
    chunkz = chunkz | 0xffffffff00000000;
    xz = chunkz & chunkx;
    return xz;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    int64_t key = getKey(x, z);
    if(!chunkMap.contains(key)) {
        chunkMap.insert(key, new Chunk(context));
    }
    *(chunkMap[key]->getBlockTypeRef(x%16, y, z%16)) = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = 0; x < 64; ++x)
    {
        for(int z = 0; z < 64; ++z)
        {
            for(int y = 0; y < 256; ++y)
            {
                if(y == 128 || y == 127 )
                {
                    if((x + z) % 2 == 0)
                    {
                        setBlockAt(x, y, z, STONE);
                    }
                    else
                    {
                        setBlockAt(x, y, z, DIRT);
                    }
                }
                else
                {
                    setBlockAt(x, y, z, EMPTY);
                }
            }
        }
    }
    // Add "walls" for collision testing
    for(int x = 0; x < 64; ++x)
    {
        setBlockAt(x, 129, 0, GRASS);
        setBlockAt(x, 130, 0, GRASS);
        setBlockAt(x, 129, 63, GRASS);
        setBlockAt(0, 130, x, GRASS);
    }
    for(int y = 129; y < 140; ++y)
    {
        setBlockAt(32, y, 32, GRASS);
    }
}
