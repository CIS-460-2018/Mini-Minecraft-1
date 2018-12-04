#pragma once

#include "scene/chunk.h"
#include <QRunnable>
#include <QMutex>

class FBMThread : public QRunnable
{
private:
    Chunk* c;
    int x;
    int z;
    int xChunk;
    int zChunk;
    float (*fbm)(float, float);
    QMutex* mutex;
public:
    FBMThread(Chunk*, int, int, int, int, float (*fbmPtr)(float, float), QMutex*);
    void run() override;
};
