#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>


MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(new Cube(this)), mp_worldAxes(new WorldAxes(this)),
      mp_progLambert(new ShaderProgram(this)), mp_progFlat(new ShaderProgram(this)),
      mp_camera(new Camera()), mp_player(new Player(mp_camera)), mp_terrain(new Terrain(this)), mp_texture(new Texture(this)),
      mp_progOverlay(new ShaderProgram(this)), overlay(new Quadrangle(this, EMPTY))
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::CrossCursor);
}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    mp_geomCube->destroy();

    delete mp_geomCube;
    delete mp_worldAxes;
    delete mp_progLambert;
    delete mp_progFlat;
    delete mp_progOverlay;
    delete mp_camera;
    delete mp_terrain;
    delete mp_player;
    delete mp_texture;
}

void MyGL::MoveMouseToCenter()
{
    QCursor::setPos(this->mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    mp_geomCube->create();
    mp_worldAxes->create();

    overlay->create();

    // Create and set up the diffuse shader
    mp_progLambert->create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    mp_progFlat->create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");
    // Create and set up the overlay shader
    mp_progOverlay->create(":/glsl/overlay.vert.glsl", ":/glsl/overlay.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
   // mp_progLambert->setGeometryColor(glm::vec4(0,1,0,1));

    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glBindVertexArray(vao);

    mp_terrain->CreateTestScene();
    mp_terrain->updateScene();
    mp_texture->create(":/minecraft_textures_all/minecraft_textures_all.png");
    mp_texture->load(0);

    startTime = QDateTime::currentMSecsSinceEpoch(); // set start time
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *mp_camera = Camera(w, h, glm::vec3(mp_player->getPosition()), //glm::vec3(mp_terrain->dimensions.x, mp_terrain->dimensions.y * 0.75, mp_terrain->dimensions.z),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progLambert->setViewVector(glm::vec4(mp_camera->look, 0));

    printGLErrorLog();
}

// MyGL's constructor links timerUpdate() to a timer that fires 60 times per second.
// We're treating MyGL as our game engine class, so we're going to use timerUpdate
void MyGL::timerUpdate()
{
    int64_t now = QDateTime::currentMSecsSinceEpoch();
    float dt = float(now - startTime); // delta-t
    // update the velocity
    dt /= 1000.0f;
    mp_player->updateVelocity();
    // check for collisions
    mp_player->checkCollision(dt, mp_terrain);
    mp_camera->RecomputeAttributes();
    startTime = now;
    mp_player->resetKey();

    mp_terrain->addChunks();

    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewVector(glm::vec4(mp_camera->look, 0));
    mp_progLambert->setTime(m_time);
    mp_progFlat->setTime(m_time);
    m_time++;
    mp_texture->bind(0);
    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progFlat->draw(*mp_worldAxes);

    glEnable(GL_DEPTH_TEST);
}

void MyGL::GLDrawScene()
{
    // first draw the opaques
    for(int64_t xz: mp_terrain->chunkMap.keys()) {
        Chunk* c = mp_terrain->chunkMap[xz];
        c->destroy();
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = xz >> 32;
        c->create();
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(xChunk*16, 0, zChunk*16)));
        mp_progLambert->draw(*c);
    }
    // then draw the transparents
    for(int64_t xz: mp_terrain->chunkMap.keys()) {
        Chunk* c = mp_terrain->chunkMap[xz];
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = xz >> 32;
        c->createTransparent();
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(xChunk*16, 0, zChunk*16)));
        mp_progLambert->drawT(*c);
    }

    BlockType inBlock = mp_terrain->getBlockAt(mp_player->position.x, mp_player->position.y, mp_player->position.z);
    overlay->destroy();
    overlay->setInBlock(inBlock);
    overlay->create();
    mp_progOverlay->drawPosNorCol(*overlay);
}

float distance3D(int x1, int y1, int z1, int x2, int y2, int z2)
{
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2) + pow(z2 - z1, 2));
}

void MyGL::removeBlock()
{
    glm::vec3 pos = mp_camera->eye;
    glm::vec3 direction = mp_camera->look;
    float t = 0.1f;
    while(t < 20) {
        glm::vec3 new_pos = pos + t * direction;
//        std::cout << new_pos.x << new_pos.y << new_pos.z << std::endl;
//        for(int x = pos.x - 1; x <= pos.x + 1; x++) {
//            for(int y = pos.y - 2; y <= pos.y + 1; y++) {
//                for(int z = pos.z - 1; z <= pos.z + 1; z++) {
//                    if(mp_terrain->getBlockAt(x, y, z) != EMPTY) {
//                        mp_terrain->setBlockAt(x, y, z, EMPTY);
//                    }
//                }
//            }
//        }
        if(mp_terrain->getBlockAt(new_pos.x, new_pos.y, new_pos.z) != EMPTY) {
            mp_terrain->setBlockAt(new_pos.x, new_pos.y, new_pos.z, EMPTY);
            break;
        }
        t += 0.1;
    }
}

void MyGL::placeBlock()
{
    glm::vec3 pos = mp_camera->eye;
    glm::vec3 direction = mp_camera->look;
    float t = 0.1f;

    while(t < 20) {
        glm::vec3 new_pos = pos + t * direction;
        if(mp_terrain->getBlockAt(new_pos.x, new_pos.y, new_pos.z) != EMPTY) {
            int x_insert = INT_MAX;
            int y_insert = INT_MAX;
            int z_insert = INT_MAX;
            float min_distance = FLT_MAX;
            for(int x = new_pos.x - 1; x <= new_pos.x + 1; x++) {
                if(mp_terrain->getBlockAt(x, new_pos.y, new_pos.z) == EMPTY) {
                    if(min_distance > distance3D(pos.x, pos.y, pos.z, x, new_pos.y, new_pos.z))
                        min_distance = distance3D(pos.x, pos.y, pos.z, x, new_pos.y, new_pos.z);
                        x_insert = x;
                        y_insert = new_pos.y;
                        z_insert = new_pos.z;
                }
            }

            for(int y = new_pos.y - 1; y <= new_pos.y + 1; y++) {
                if(mp_terrain->getBlockAt(new_pos.x, y, new_pos.z) == EMPTY) {
                    if(min_distance > distance3D(pos.x, pos.y, pos.z, new_pos.x, y, new_pos.z))
                        min_distance = distance3D(pos.x, pos.y, pos.z, new_pos.x, y, new_pos.z);
                        x_insert = new_pos.x;
                        y_insert = y;
                        z_insert = new_pos.z;
                }
            }

            for(int z = new_pos.z - 1; z <= new_pos.z + 1; z++) {
                if(mp_terrain->getBlockAt(new_pos.x, new_pos.y, z) == EMPTY) {
                    if(min_distance > distance3D(pos.x, pos.y, pos.z, new_pos.x, new_pos.y, z))
                        min_distance = distance3D(pos.x, pos.y, pos.z, new_pos.x, new_pos.y, z);
                        x_insert = new_pos.x;
                        y_insert = new_pos.y;
                        z_insert = z;
                }
            }

            if(min_distance != FLT_MAX) {
                mp_terrain->setBlockAt(x_insert, y_insert, z_insert, LAVA);
            }
            break;
        }
        t += 0.1;
    }
}

//Can make this more efficient by replacing createTestScene with a function that only
//renders the quadrant you need rendered rather than the whole scene again
void MyGL::checkBoundary()
{

    glm::vec3 pos = mp_camera->eye;
    int radius = 2;
    for(int numChunksX = -radius; numChunksX < 1 + radius*2; numChunksX++) {
        for(int numChunksZ = -radius; numChunksZ < 1 + radius*2; numChunksZ++) {
            if(!mp_terrain->hasChunk(pos.x + numChunksX*16, pos.z + numChunksZ*16)) {
                glm::vec3 newPos = glm::vec3(pos.x + numChunksX*16, pos.y, pos.z + numChunksZ*16);
                mp_terrain->createNewChunk(newPos);
            }
        }
    }
}

void MyGL::mousePressEvent(QMouseEvent *e)
{
    if(e->buttons() == Qt::LeftButton) {
        removeBlock();
    } else if(e->buttons() == Qt::RightButton) {
        placeBlock();
    }
    mp_terrain->updateChunk(mp_camera->eye);
}

void MyGL::mouseMoveEvent(QMouseEvent *e)
{
    mp_player->updateMouse(e);
    glm::vec2 pos(e->pos().x(), e->pos().y());
    glm::vec2 center(width() / 2, height() / 2);
    glm::vec2 diff = 0.4f * (pos - center);
    mp_camera->RotateAboutUp(-diff.x);
    mp_camera->RotateAboutRight(-diff.y);
    mp_camera->RecomputeAttributes();
    MoveMouseToCenter();
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    mp_player->updateKey(e);
    checkBoundary();
}

