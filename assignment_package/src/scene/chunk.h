#pragma once

#include "drawable.h"
#include "la.h"
#include <scene/terrain.h>
#include "blocktype.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

using namespace std;

class Chunk: public Drawable
{
private:
    void createVertexPosNorCol(vector<glm::vec4> &posNorCol);
    void createIndices(vector<GLuint> &idx);
public:
    int faces;
    vector<glm::vec4> c_vert_pos_nor_col;
    BlockType m_blocks[65536];
    Chunk(OpenGLContext* context);
    Chunk(OpenGLContext* context, BlockType blocks[16][256][16]);
    virtual ~Chunk() override {}

    BlockType getBlockType(int x, int y, int z) const;
    BlockType* getBlockTypeRef(int x, int y, int z);

    void create() override;
};
