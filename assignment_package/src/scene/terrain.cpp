#include <scene/terrain.h>
#include <scene/cube.h>
#include <random>
#include <iostream>
#include <math.h>
#include "vbothread.h"
#include <QThreadPool>

using namespace glm;
using namespace std;

Terrain::Terrain(OpenGLContext* c)
    : context(c), dimensions(64, 256, 64), x_boundary_start(-192), y_boundary_start(0), z_boundary_start(-128),
      x_boundary_end(128), y_boundary_end(256), z_boundary_end(256)
{}

Terrain::Terrain(OpenGLContext* c, int x_boundary_end, int y_boundary_end, int z_boundary_end)
    : context(c), dimensions(x_boundary_end, y_boundary_end, z_boundary_end),
      x_boundary_start(0), x_boundary_end(x_boundary_end), y_boundary_start(0), y_boundary_end(y_boundary_end),
      z_boundary_start(0), z_boundary_end(z_boundary_end)
{}

float rand(vec2 n) {
    return (fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453));
}

float interpNoise2D(float x, float y) {
    float intX = floor(x);
    float fractX = fract(x);
    float intY = floor(y);
    float fractY = fract(y);

    float v1 = rand(vec2(intX, intY));
    float v2 = rand(vec2(intX + 1, intY));
    float v3 = rand(vec2(intX, intY + 1));
    float v4 = rand(vec2(intX + 1, intY + 1));

    float i1 = mix(v1, v2, fractX);
    float i2 = mix(v3, v4, fractX);

    return mix(i1, i2, fractY);
}

float fbm(float x, float y) {
    x /= 64.f; y /= 64.f;
    float total = 0;
    float persistence = 0.45f;
    int octaves = 8;

    for(int i = 1; i<= octaves; i++) {
        float freq = pow(2.f, i);
        float amp = pow(persistence, i);

        total += interpNoise2D(x * freq, y * freq) * amp;
    }

    return total;
}

BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    int64_t key = getKey(x, z, false);
    int originalX = x;
    int originalZ = z;
    x = abs(x)%16;
    z = abs(z)%16;
    if(originalX < 0 && x != 0) {
        x = 16 - x;
    }
    if(originalZ < 0 && z != 0) {
        z = 16 - z;
    }
    if(!chunkMap.contains(key)) {
        return EMPTY;
    }
    return chunkMap[key]->getBlockType(x, y, z);
}

int64_t Terrain::getKey(int x, int z, bool chunked) const {
    int64_t xz = 0xffffffffffffffff;
    int64_t chunkx;
    int64_t chunkz;
    if(chunked) {
        chunkx = x;
        chunkz = z;
    } else {
        float fx = floorf(x / 16.0);
        float fz = floorf(z / 16.0);
        chunkx = int64_t(fx);
        chunkz = int64_t(fz);
    }
    xz = (xz & (chunkx << 32)) | 0x00000000ffffffff;
    chunkz = chunkz | 0xffffffff00000000;
    xz = xz & chunkz;
    return xz;
}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    int64_t key = getKey(x, z, false);
    if(!chunkMap.contains(key)) {
        chunkMap.insert(key, new Chunk(context));
    }
    int originalX = x;
    int originalZ = z;
    x = abs(x)%16;
    z = abs(z)%16;
    if(originalX < 0 && x != 0) {
        x = 16 - x;
    }
    if(originalZ < 0 && z != 0) {
        z = 16 - z;
    }
    *(chunkMap[key]->getBlockTypeRef(x, y, z)) = t;
}

void Terrain::CreateTestScene()
{
    // Create the basic terrain floor
    for(int x = x_boundary_start; x < x_boundary_end; ++x)
    {
        for(int z = z_boundary_start; z < z_boundary_end; ++z)
        {
            float height = fbm(x, z);

            height = 128 + height * 32;
            //height = 116 + height * 10;

            if (height < 128) {
                height = 128.f;
            }
            else if (height > 256) {
                height = 256.f;
            }
            for(int y = 0; y < 256; y++) {
                if(y < height) {
                    if(y == ceil(height) - 1) {
                        setBlockAt(x, y, z, GRASS);
                    }
                    else if(y >= 128) {
                        setBlockAt(x, y, z, DIRT);
                    }
                    else {
                        setBlockAt(x, y, z, STONE);
                    }
                } else {
                    setBlockAt(x, y, z, EMPTY);
                }
            }
        }
    }

    //L-System generation
    this->l_system_delta = new LSystem(QString("FFFX"), x_boundary_start, x_boundary_end, -64, z_boundary_end);
    drawLSystem(l_system_delta);

    this->l_system_linear = new LSystem(QString("FFFFFY"), x_boundary_start, x_boundary_end, 100, z_boundary_end);
    drawLSystem(l_system_linear);

}

void Terrain::drawLSystem(LSystem *l_system) {
    //Expanding the axiom for n iterations
    for(int i = 0; i < 10; i++) {
        l_system->axiom = l_system->expandGrammar(l_system->axiom);
    }

    //Match rules to each character in the axiom (defined in lsystems.cpp)
    int count = 0;
    while (count < l_system->axiom.length()) {
        l_system->executeRule(l_system->axiom.at(count), count);
        count = count + 1;
    }

    //Trace the turtle's route and update blocks correspondingly from the states stored in the turtleHistory stack
    Turtle start = l_system->turtleHistory.first();
    l_system->turtleHistory.pop_front();
    count = l_system->turtleHistory.size() - 1;
    while(count > 0) {
        Turtle nextTurtle = l_system->turtleHistory.first();
        l_system->turtleHistory.pop_front();
        if(hasChunk(nextTurtle.pos.x, nextTurtle.pos.y) && hasChunk(start.pos.x, start.pos.y))
        {
            //Only drawRoute if depth of next turtle is 1 more than start turtle
            //This facilitates branching logic and prevents rotations from being drawn
            if(nextTurtle.depth == start.depth + 1)
            {
                drawRoute(start, nextTurtle);
            }
        }
        else if(hasChunk(start.pos.x, start.pos.y)) {
            if(nextTurtle.depth == start.depth + 1)
            {
                drawRoute(start, nextTurtle);
            }
            l_system->turtleHistory.push_back(start);
            l_system->turtleHistory.push_back(nextTurtle);
        }
        else {
            l_system->turtleHistory.push_back(nextTurtle);
        }
//        drawRoute(start, nextTurtle);
        start = nextTurtle;
        count = count - 1;
    }

//    std::cout << l_system->turtleHistory.size() << std::endl;
}

void Terrain::drawRemainderLSystem(LSystem *l_system, Chunk* c) {
    if(l_system->turtleHistory.size() > 0)
    //Trace the turtle's route and update blocks correspondingly from the states stored in the turtleHistory stack
    {
//        std::cout << "Entered" << std::endl;
        Turtle start = l_system->turtleHistory.first();
        l_system->turtleHistory.pop_front();
        int count = l_system->turtleHistory.size() - 1;
        while(count > 0) {
            Turtle nextTurtle = l_system->turtleHistory.first();
            l_system->turtleHistory.pop_front();
            if(hasChunk(nextTurtle.pos.x, nextTurtle.pos.y) && hasChunk(start.pos.x, start.pos.y))
            {
//                std::cout << "Entered has chunk" << std::endl;
                //Only drawRoute if depth of next turtle is 1 more than start turtle
                //This facilitates branching logic and prevents rotations from being drawn
                if(nextTurtle.depth == start.depth + 1)
                {
//                    std::cout << "Entered draw route" << std::endl;
                    drawRemainderRoute(start, nextTurtle, c);
                }
            }
            else if(hasChunk(start.pos.x, start.pos.y)) {
                if(nextTurtle.depth == start.depth + 1)
                {
//                    std::cout << "Entered draw route" << std::endl;
                    drawRemainderRoute(start, nextTurtle, c);
                }
                l_system->turtleHistory.push_back(start);
                l_system->turtleHistory.push_back(nextTurtle);
            }
            else {
                l_system->turtleHistory.push_back(nextTurtle);
            }
            start = nextTurtle;
            count = count - 1;
        }
    }
}

bool Terrain::withinChunks(const int x, const int z) {
    return true;
}

void Terrain::drawRoute(Turtle startTurtle, Turtle nextTurtle) {
    int start_x = startTurtle.pos.x;
    int end_x = nextTurtle.pos.x;
    int start_z = startTurtle.pos.y;
    int end_z = nextTurtle.pos.y;
    float distance = sqrt(pow(end_x - start_x, 2) + pow(end_z - start_z, 2));
    float x_incr = (end_x - start_x) / distance;
    float z_incr = (end_z - start_z) / distance;
    int width = std::max(7 - nextTurtle.depth/3, 3);

    for(int i = 0; i <= distance; i++) {
        //Check within boundary that has been rendered
//        if(start_x + (i * x_incr) < x_boundary_end && start_x + (i * x_incr) > x_boundary_start && start_z + (i * z_incr) < z_boundary_end && start_z + (i * z_incr) > z_boundary_start) {
        if(hasChunk(int(start_x + (i * x_incr)),  int(start_z + (i * z_incr)))) {
//            std::cout << "Drawing" << std::endl;
            //Increment x and z values by the width and setBlockAt those positions as well to give the river some thickness
            for(int d = -width; d <= width; d++) {
//                if(start_x + (i * x_incr) + d < x_boundary_end && start_x + (i * x_incr) + d > x_boundary_start && start_z + (i * z_incr) + d < z_boundary_end && start_z + (i * z_incr) + d > z_boundary_start) {
                if(hasChunk(int(start_x + (i * x_incr) + d),  int(start_z + (i * z_incr) + d))) {
//                    std::cout << "Setting blocks" << std::endl;
                    setBlockAt(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr), WATER);
                    setBlockAt(start_x + (i * x_incr), 128, start_z + (i * z_incr) + d, WATER);
                    setBlockAt(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr) + d, WATER);

                    //Set all blocks above the river to be empty
                    for(int y = 129; y < 256; y++) {
                        setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr), EMPTY);
                        setBlockAt(start_x + (i * x_incr), y, start_z + (i * z_incr) + d, EMPTY);
                        setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr) + d, EMPTY);
                    }
                }
            }

            //To smooth edges of the river
//            for(int d = -20; d <= 20; d++) {
//            for(int d = std::min(-width*3, -15); d <= std::max(width*3, 15); d++) {
            for(int d = -width*3; d <= 3*width; d++){
                if(d < -width || d > width)
                {
//                    if(start_x + (i * x_incr) + d < x_boundary_end && start_x + (i * x_incr) + d > x_boundary_start && start_z + (i * z_incr) + d < z_boundary_end && start_z + (i * z_incr) + d > z_boundary_start) {
                    if(hasChunk(int(start_x + (i * x_incr) + d),  int(start_z + (i * z_incr) + d))) {
                    for(int y = 129 + fabs(d) - width; y < 256; y++) {
                            setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr), EMPTY);
                            setBlockAt(start_x + (i * x_incr), y, start_z + (i * z_incr) + d, EMPTY);
                            setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr) + d, EMPTY);
                        }
                    }
                }
            }


        }
    }//for each increment of distance from start to next turtle

}

void Terrain::drawRemainderRoute(Turtle startTurtle, Turtle nextTurtle, Chunk* c) {
    int start_x = startTurtle.pos.x;
    int end_x = nextTurtle.pos.x;
    int start_z = startTurtle.pos.y;
    int end_z = nextTurtle.pos.y;
    float distance = sqrt(pow(end_x - start_x, 2) + pow(end_z - start_z, 2));
    float x_incr = (end_x - start_x) / distance;
    float z_incr = (end_z - start_z) / distance;
    int width = std::max(7 - nextTurtle.depth/3, 3);

    std::cout << "Entered drawRemainderRoute" << std::endl;

    for(int i = 0; i <= distance; i++) {
        //Check within boundary that has been rendered
//        if(start_x + (i * x_incr) < x_boundary_end && start_x + (i * x_incr) > x_boundary_start && start_z + (i * z_incr) < z_boundary_end && start_z + (i * z_incr) > z_boundary_start) {
        if(hasChunk(int(start_x + (i * x_incr)),  int(start_z + (i * z_incr)))) {
//            std::cout << "Drawing" << std::endl;
            //Increment x and z values by the width and setBlockAt those positions as well to give the river some thickness
            for(int d = 0; d < 16; d++) {
//                if(start_x + (i * x_incr) + d < x_boundary_end && start_x + (i * x_incr) + d > x_boundary_start && start_z + (i * z_incr) + d < z_boundary_end && start_z + (i * z_incr) + d > z_boundary_start) {
                if(hasChunk(int(start_x + (i * x_incr) + d),  int(start_z + (i * z_incr) + d))) {
//                    std::cout << "Setting blocks" << std::endl;
//                    setBlockAt(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr), WATER);
//                    setBlockAt(start_x + (i * x_incr), 128, start_z + (i * z_incr) + d, WATER);
//                    setBlockAt(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr) + d, WATER);
//                    std::cout << "Before setting blocks" << std::endl;
                    int64_t xz1 = getKey(start_x + (i * x_incr) + d,start_z + (i * z_incr), false);
                    Chunk* c1 = chunkMap[xz1];
                    int64_t xz2 = getKey(start_x + (i * x_incr), start_z + (i * z_incr) + d, false);
                    Chunk* c2 = chunkMap[xz2];
                    int64_t xz3 = getKey(start_x + (i * x_incr) + d, start_z + (i * z_incr) + d, false);
                    Chunk* c3 = chunkMap[xz3];

//                    *(c1->getBlockTypeRef(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr))) = WATER;
//                    *(c2->getBlockTypeRef(start_x + (i * x_incr), 128, start_z + (i * z_incr) + d)) = WATER;
//                    *(c3->getBlockTypeRef(start_x + (i * x_incr) + d, 128, start_z + (i * z_incr) + d)) = WATER;
//                    *(c1->getBlockTypeRef(0+d, 128, 0)) = WATER;
//                    *(c2->getBlockTypeRef(0, 128, 0+d)) = WATER;
//                    *(c3->getBlockTypeRef(0+d, 128, 0+d)) = WATER;
//                    std::cout << "Setting chunk" << std::endl;
                    *(c->getBlockTypeRef(0+d, 128, 0+d)) = WATER;

                    //Set all blocks above the river to be empty
                    for(int y = 129; y < 256; y++) {
//                        setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr), EMPTY);
//                        setBlockAt(start_x + (i * x_incr), y, start_z + (i * z_incr) + d, EMPTY);
//                        setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr) + d, EMPTY);

//                        *(c1->getBlockTypeRef(start_x + (i * x_incr) + d, y, start_z + (i * z_incr))) = EMPTY;
//                        *(c2->getBlockTypeRef(start_x + (i * x_incr), y, start_z + (i * z_incr) + d)) = EMPTY;
//                        *(c3->getBlockTypeRef(start_x + (i * x_incr) + d, y, start_z + (i * z_incr) + d)) = EMPTY;
//                        *(c1->getBlockTypeRef(0+d, y, 0)) = EMPTY;
//                        *(c2->getBlockTypeRef(0, y, 0+d)) = EMPTY;
//                        *(c3->getBlockTypeRef(0+d, y, 0+d)) = EMPTY;
                        *(c->getBlockTypeRef(0+d, y, 0+d)) = EMPTY;
                    }
                }
            }

            //To smooth edges of the river
//            for(int d = -20; d <= 20; d++) {
//            for(int d = std::min(-width*3, -15); d <= std::max(width*3, 15); d++) {
//            for(int d = -width*3; d <= 3*width; d++){
//                if(d < -width || d > width)
//                {
////                    if(start_x + (i * x_incr) + d < x_boundary_end && start_x + (i * x_incr) + d > x_boundary_start && start_z + (i * z_incr) + d < z_boundary_end && start_z + (i * z_incr) + d > z_boundary_start) {
//                    if(hasChunk(int(start_x + (i * x_incr) + d),  int(start_z + (i * z_incr) + d))) {
//                    for(int y = 129 + fabs(d) - width; y < 256; y++) {
//                            setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr), EMPTY);
//                            setBlockAt(start_x + (i * x_incr), y, start_z + (i * z_incr) + d, EMPTY);
//                            setBlockAt(start_x + (i * x_incr) + d, y, start_z + (i * z_incr) + d, EMPTY);
//                        }
//                    }
//                }
//            }


        }
    }//for each increment of distance from start to next turtle

}

void Terrain::createNewChunk(glm::vec3 position) {
    int64_t xz = getKey(position.x, position.z, false);
    int64_t zChunk = xz & 0x00000000ffffffff;
    if(zChunk & 0x0000000080000000) {
        zChunk = zChunk | 0xffffffff00000000;
    }
    int64_t xChunk = (xz >> 32);
    Chunk* c = new Chunk(context);
    for(int x = 0; x < 16; ++x)
    {
        for(int z = 0; z < 16; ++z)
        {
            float height = fbm(xChunk*16 + x, zChunk*16 + z);
            height = 128 + height * 32;
            //height = 116 + height * 10;

            if (height < 128) {
                height = 128.f;
            }
            else if (height > 256) {
                height = 256.f;
            }
            for(int y = 0; y < 256; y++) {
                if(y < height) {
                    if(y == ceil(height) - 1) {
                        *(c->getBlockTypeRef(x, y, z)) = GRASS;
                    }
                    else if(y >= 128) {
                        *(c->getBlockTypeRef(x, y, z)) = DIRT;
                    }
                    else {
                        *(c->getBlockTypeRef(x, y, z)) = STONE;
                    }
                } else {
                    *(c->getBlockTypeRef(x, y, z)) = EMPTY;
                }
            }
        }
    }

    drawRemainderLSystem(this->l_system_delta, c);
    drawRemainderLSystem(this->l_system_linear, c);


    pair<int, int> ints ((int)xChunk, (int)zChunk);
    pair<pair<int, int>, Chunk*> p(ints, c);
    chunksToAdd.push_back(p);


//    //L-System generation
//    LSystem *l_system_delta = new LSystem(QString("FFFX"), x_boundary_start, x_boundary_end, -64, z_boundary_end);
//    drawLSystem(l_system_delta);

//    LSystem *l_system_linear = new LSystem(QString("FFFFFY"), x_boundary_start, x_boundary_end, 100, z_boundary_end);
//    drawLSystem(l_system_linear);

//    drawRemainderLSystem(this->l_system_delta);
//    drawRemainderLSystem(this->l_system_linear;

//    CreateTestScene();
}


void Terrain::updateChunk(glm::vec3 position) {
    QMutex mutex;
    int64_t xz = getKey(position.x, position.z, false);
    Chunk* c = chunkMap[xz];
    c->destroy();
    int64_t zChunk = xz & 0x00000000ffffffff;
    if(zChunk & 0x0000000080000000) {
        zChunk = zChunk | 0xffffffff00000000;
    }
    int64_t xChunk = (xz >> 32);
    VBOThread* thread = new VBOThread(c, (int)xChunk, (int)zChunk, chunkMap, &mutex);
    QThreadPool::globalInstance()->start(thread);
    QThreadPool::globalInstance()->waitForDone();
    c->create();
}

void Terrain::updateScene() {
    QMutex mutex;
    for(int64_t xz: chunkMap.keys()) {
        Chunk* c = chunkMap[xz];
        c->destroy();
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = (xz >> 32);
        VBOThread* thread = new VBOThread(c, (int)xChunk, (int)zChunk, chunkMap, &mutex);
        QThreadPool::globalInstance()->start(thread);
    }
    QThreadPool::globalInstance()->waitForDone();
    for(int64_t xz: chunkMap.keys()) {
        Chunk* c = chunkMap[xz];
        c->create();
    }
}

void Terrain::addChunks() {
    QMutex mutex;
    for(pair<pair<int, int>, Chunk*> p: chunksToAdd) {
        VBOThread* thread = new VBOThread(p.second, p.first.first, p.first.second, chunkMap, &mutex);
        QThreadPool::globalInstance()->start(thread);
    }
    QThreadPool::globalInstance()->waitForDone();
    for(pair<pair<int, int>, Chunk*> p: chunksToAdd) {
        chunkMap[getKey(p.first.first, p.first.second, true)] = p.second;
    }
    chunksToAdd.clear();
}

bool Terrain::hasChunk(int x, int z) {
    int64_t key = getKey(x, z, false);
    return chunkMap.contains(key);
}
