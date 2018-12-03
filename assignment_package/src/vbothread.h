#pragma once

#include "scene/chunk.h"
#include <QRunnable>
#include <QMutex>

class VBOThread : public QRunnable
{
private:
    Chunk* c;
    int xChunk;
    int zChunk;
    BlockType currT;
    QHash<int64_t, Chunk*> chunkMap;
    QMutex* mutex;
    int uvIndicator;
public:
    VBOThread(Chunk*, int, int, QHash<int64_t, Chunk*>, QMutex*);
    void run() override;

    void createVertexPosNorUV();
    glm::vec2 getTexture(int faceNum);
    bool checkEmpty(int, int, int, bool (VBOThread::*checkFunc)(BlockType));
    bool shouldRenderSolid(BlockType);
    bool shouldRenderTransp(BlockType);
    int64_t getKey(int, int, bool) const;
};
