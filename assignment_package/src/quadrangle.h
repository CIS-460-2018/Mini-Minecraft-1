#pragma once

#include "drawable.h"
#include <la.h>
#include "blocktype.h"

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Quadrangle : public Drawable
{
private:
    BlockType inBlock;
    glm::vec4 vert_pos_nor_col[12];
public:
    Quadrangle(OpenGLContext* context, BlockType b) : Drawable(context), inBlock(b){}
    virtual ~Quadrangle(){}
    void create() override;
    void setInBlock(BlockType t);
};
