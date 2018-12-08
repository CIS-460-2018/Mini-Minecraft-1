#ifndef CURSOR_H
#define CURSOR_H

#include "drawable.h"
#include "camera.h"
#include <la.h>

#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Cursor : public Drawable
{
private:
    glm::vec4 center;
    glm::vec4 right;
    glm::vec4 up;
public:
    Cursor(OpenGLContext* context) : Drawable(context){}
    virtual ~Cursor(){}
    void create() override;
    GLenum drawMode() override;
};

#endif // CURSOR_H
