#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <QDateTime>
#include<QFileDialog>
#include<QImage>
#include<QRgb>

using namespace std;

MyGL::MyGL(QWidget *parent)
    : OpenGLContext(parent),
      mp_geomCube(new Cube(this)), mp_worldAxes(new WorldAxes(this)),
      mp_progLambert(new ShaderProgram(this)), mp_progFlat(new ShaderProgram(this)),
      mp_camera(new Camera()), mp_terrain(new Terrain(this)), mp_player(new Player(mp_camera)), mp_texture(new Texture(this)),
      mp_progOverlay(new ShaderProgram(this)), overlay(new Quadrangle(this, EMPTY)), cur(new Cursor(this)),
      sheepTexture(new Texture(this))//, mp_sheep(new NPC(mp_terrain, this))
{
    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

    setMouseTracking(true); // MyGL will track the mouse's movements even if a mouse button is not pressed
    setCursor(Qt::BlankCursor); // Make the cursor invisible
    for (int i = 0; i < 3; i++) {
        NPC* newSheep = new NPC(mp_terrain, this);
        newSheep->generatePosition();
        mp_sheep.push_back(newSheep);
    }
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
    delete sheepTexture;
    delete cur;
    delete overlay;
    //delete mp_sheep;
    for (int i = 0; i < 3; i++) {
        delete mp_sheep[i];
    }
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
    glGenFramebuffers(1, &m_frameBuffer);
    glGenTextures(1, &m_renderedTexture);
    glGenRenderbuffers(1, &m_depthRenderBuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Initialize our depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderBuffer);
    // Set m_renderedTexture as the color output of our frame buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_renderedTexture, 0);

    // Sets the color output of the fragment shader to be stored in GL_COLOR_ATTACHMENT0, which we previously set to m_renderedTextures[i]
    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers); // "1" is the size of drawBuffers

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Frame buffer did not initialize correctly..." << std::endl;
        printGLErrorLog();
    }

    //Create the instance of Cube
    mp_geomCube->create();
    mp_worldAxes->create();
    cur->create();
    overlay->create();
    //mp_sheep->create();
    for (int i = 0; i < 3; i++) {
        mp_sheep[i]->create();
    }

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

    mp_terrain->initializeBiomeMap();
    mp_terrain->CreateTestScene();
    mp_terrain->updateScene();
    mp_texture->create(":/minecraft_textures_all/minecraft_textures_all.png");
    sheepTexture->create(":/minecraft_textures_all/sheep.png");
    mp_texture->load(0);
    sheepTexture->load(1);

    startTime = QDateTime::currentMSecsSinceEpoch(); // set start time
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
    *mp_camera = Camera(w, h, glm::vec3(mp_player->getPosition()),
                       glm::vec3(mp_terrain->dimensions.x / 2, mp_terrain->dimensions.y / 2, mp_terrain->dimensions.z / 2), glm::vec3(0,1,0));
    glm::mat4 viewproj = mp_camera->getViewProj();

    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    mp_progLambert->setViewProjMatrix(viewproj);
    mp_progFlat->setViewProjMatrix(viewproj);
    mp_progLambert->setViewVector(glm::vec4(mp_camera->look, 0));
    mp_progOverlay->setDimensions(glm::ivec2(w, h));
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Bind our texture so that all functions that deal with textures will interact with this one
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    // Give an empty image to OpenGL ( the last "0" )
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio(), 0, GL_RGB, GL_UNSIGNED_BYTE, (void*)0);
    // Set the render settings for the texture we've just created.
    // Essentially zero filtering on the "texture" so it appears exactly as rendered
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // Clamp the colors at the edge of our texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

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
    for (int i = 0; i < 3; i++) {
        mp_sheep[i]->updateVelocity();
        mp_sheep[i]->checkCollision(dt);
    }
    //mp_sheep->updateVelocity();
    // check for collisions
    mp_player->checkCollision(dt, mp_terrain);
    //mp_sheep->checkCollision(dt);
    mp_camera->RecomputeAttributes();
    startTime = now;
    //mp_player->resetKey();

    mp_terrain->addChunks();

    update();
}

// This function is called whenever update() is called.
// MyGL's constructor links update() to a timer that fires 60 times per second,
// so paintGL() called at a rate of 60 frames per second.
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);

    mp_progFlat->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewProjMatrix(mp_camera->getViewProj());
    mp_progLambert->setViewVector(glm::vec4(mp_camera->look, 0));
    mp_progLambert->setPlayerPos(mp_player->getPosition());
    mp_progLambert->setTime(m_time);
    mp_progFlat->setTime(m_time);
    mp_progOverlay->setTime(m_time);
    m_time++;

    mp_texture->bind(0);
    sheepTexture->bind(1);
    mp_progLambert->setModelMatrix(glm::mat4());
    for (int i = 0; i < 3; i++) {
        mp_sheep[i]->destroy();
        mp_sheep[i]->create();
        mp_progLambert->draw(*(mp_sheep[i]), 1);
    }
    GLDrawScene();

    glDisable(GL_DEPTH_TEST);
    mp_progFlat->setModelMatrix(glm::mat4());
    mp_progFlat->drawPosNorCol(*mp_worldAxes);
    mp_progFlat->setViewProjMatrix(glm::mat4());
    mp_progFlat->drawPosNorCol(*cur);
    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, this->defaultFramebufferObject());
    // Render on the whole framebuffer, complete from the lower left corner to the upper right
    glViewport(0,0,this->width() * this->devicePixelRatio(), this->height() * this->devicePixelRatio());
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_renderedTexture);
    if (mp_terrain->getBlockAt(mp_player->getPosition().x, mp_player->getPosition().y, mp_player->getPosition().z) == WATER) {
        mp_progOverlay->setUnifMode(1);
    } else if (mp_terrain->getBlockAt(mp_player->getPosition().x, mp_player->getPosition().y, mp_player->getPosition().z) == LAVA) {
        mp_progOverlay->setUnifMode(2);
    } else {
        mp_progOverlay->setUnifMode(0);
    }
    mp_progOverlay->drawOverlay(*overlay);
}

void MyGL::GLDrawScene()
{
    // first draw the opaques
    for(int64_t xz: mp_terrain->chunkMap.keys()) {
        Chunk* c = mp_terrain->chunkMap[xz];
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = xz >> 32;
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(xChunk*16, 0, zChunk*16)));
        mp_progLambert->draw(*c, 0);
    }
    // then draw the transparents
    for(int64_t xz: mp_terrain->chunkMap.keys()) {
        Chunk* c = mp_terrain->chunkMap[xz];
        int64_t zChunk = xz & 0x00000000ffffffff;
        if(zChunk & 0x0000000080000000) {
            zChunk = zChunk | 0xffffffff00000000;
        }
        int64_t xChunk = xz >> 32;
        mp_progLambert->setModelMatrix(glm::translate(glm::mat4(), glm::vec3(xChunk*16, 0, zChunk*16)));
        mp_progLambert->drawT(*c);
    }

    BlockType inBlock = mp_terrain->getBlockAt(mp_player->position.x, mp_player->position.y, mp_player->position.z);
    if(overlay->inBlock != inBlock) {
        overlay->setInBlock(inBlock);
        overlay->destroy();
        overlay->create();
    }
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
    if((e->key() != 0) && (e->key() == Qt::Key_G)) {
        QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Choose greyscale image"),
                                                    "/",
                                                    tr("Image Files (*.png *.jpg *.bmp)"));
        if(fileName.length() > 1) {
            QImage* img = new QImage(fileName);
            int sizeLimit = 256;
            *img = img->scaled(QSize(sizeLimit, sizeLimit), Qt::KeepAspectRatio, Qt::FastTransformation);
            int w = img->width();
            int h = img->height();

            vector<vector<float>> newHeight;
            vector<vector<pair<float, BlockType>>> newBlocks;
            vector<pair<glm::vec3, BlockType>> colorMap;
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(25, 25, 25), BLACK));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(62, 62, 62), DARK_GRAY));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(155, 50, 46), RED));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(215, 120, 150), PINK));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(55, 65, 28), DARK_GREEN));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(70, 180, 55), GREEN));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(78, 50, 30), BROWN));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(160, 170, 45), YELLOW));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(45, 50, 145), DARK_BLUE));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(120, 130, 205), BLUE));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(125, 65, 165), PURPLE));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(180, 80, 185), MAGENTA));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(45, 110, 138), TURQOUISE));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(215, 120, 50), ORANGE));
            colorMap.push_back(pair<glm::vec3, BlockType>(glm::vec3(160, 160, 160), GRAY));

            bool greyscale = img->allGray();
            float pixCounter = 0;
            for(int i = 0; i < w; i++) {
                newHeight.push_back(vector<float>());
                newBlocks.push_back(vector<pair<float, BlockType>>());
                for(float j = 0; j < h; j++) {
                    QColor p = img->pixel(i, j);
                    if(greyscale) {
                        newHeight[i].push_back(p.red() / 255.f * 32.f + 127);
                    } else {
                        float gVal = .2126 * p.red() + .7152 * p.green() + .0722 * p.blue();
                        glm::vec3 currCol = glm::vec3(p.red(), p.green(), p.blue());
                        BlockType b = BLACK;
                        double minDist = std::sqrt(pow(currCol.x - colorMap[0].first.x, 2) +
                                                   pow(currCol.y - colorMap[0].first.y, 2) +
                                                   pow(currCol.z - colorMap[0].first.z, 2));
                        for(int c = 0; c < colorMap.size(); c++) {
                            double newDist = std::sqrt(pow(currCol.x - colorMap[c].first.x, 2) +
                                                       pow(currCol.y - colorMap[c].first.y, 2) +
                                                       pow(currCol.z - colorMap[c].first.z, 2));
                            if(newDist < minDist) {
                                newDist = minDist;
                                b = colorMap[c].second;
                            }
                        }
                        newBlocks[i].push_back(pair<float, BlockType>(gVal / 255.f * 32.f + 127, b));
                    }
                }
            }
            if(greyscale) {
                mp_terrain->updatePictureArea(mp_player->getPosition().x, mp_player->getPosition().z, newHeight);
            } else {
                mp_terrain->updateColorPictureArea(mp_player->getPosition().x, mp_player->getPosition().z, newBlocks);
            }
            mp_terrain->updateScene();
        }
    } else {
        mp_player->updateKey(e);
        checkBoundary();
    }
}

void MyGL::keyReleaseEvent(QKeyEvent *event)
{
    mp_player->stopFootstep();
    mp_player->updateKey(event);
    mp_player->resetKey();
}

