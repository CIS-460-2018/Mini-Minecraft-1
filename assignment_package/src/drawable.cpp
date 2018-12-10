#include "drawable.h"
#include <la.h>

Drawable::Drawable(OpenGLContext* context)
    : bufIdx(), bufIdxT(), bufPosNorUVOpaque(), bufPosNorUVTransparent(),
      idxBound(false), idxBoundT(false), posNorUVOpaqueBound(false), posNorUVTransparentBound(false),
      bufPos(), bufNor(), bufUV(), posBound(false), norBound(false), uvBound(false),
      context(context)
{}

Drawable::~Drawable()
{}


void Drawable::destroy()
{
    context->glDeleteBuffers(1, &bufIdx);
    context->glDeleteBuffers(1, &bufIdxT);
    context->glDeleteBuffers(1, &bufPosNorUVOpaque);
    context->glDeleteBuffers(1, &bufPosNorUVTransparent);
    context->glDeleteBuffers(1, &bufPos);
    context->glDeleteBuffers(1, &bufNor);
    context->glDeleteBuffers(1, &bufUV);
}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCount()
{
    return count;
}

int Drawable::elemCountT()
{
    return countT;
}

void Drawable::generateIdx()
{
    idxBound = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdx);
}

void Drawable::generateIdxT()
{
    idxBoundT = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxT);
}

void Drawable::generateIdxPosNorCol()
{
    idxBoundCol = true;
    // Create a VBO on our GPU and store its handle in bufIdx
    context->glGenBuffers(1, &bufIdxCol);
}

void Drawable::generatePosNorUVOpaque()
{
    posNorUVOpaqueBound = true;
    // Create a VBO on our GPU and store its handle in bufPosNorUVOpaque
    context->glGenBuffers(1, &bufPosNorUVOpaque);
}

void Drawable::generatePosNorUVTransparent()
{
    posNorUVTransparentBound = true;
    // Create a VBO on our GPU and store its handle in bufPosNorUVTransparent
    context->glGenBuffers(1, &bufPosNorUVTransparent);
}

void Drawable::generatePosNorCol()
{
    posNorColBound = true;
    // Create a VBO on our GPU and store its handle in bufPosNorUVTransparent
    context->glGenBuffers(1, &bufPosNorCol);
}

void Drawable::generatePos()
{
    posBound = true;
    // Create a VBO on our GPU and store its handle in bufPos
    context->glGenBuffers(1, &bufPos);
}

void Drawable::generateNor()
{
    norBound = true;
    // Create a VBO on our GPU and store its handle in bufNor
    context->glGenBuffers(1, &bufNor);
}

void Drawable::generateUV()
{
    uvBound = true;
    // Create a VBO on our GPU and store its handle in bufCol
    context->glGenBuffers(1, &bufUV);
}

bool Drawable::bindIdx()
{
    if(idxBound) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdx);
    }
    return idxBound;
}

bool Drawable::bindIdxT()
{
    if(idxBoundT) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxT);
    }
    return idxBoundT;
}

bool Drawable::bindIdxCol()
{
    if(idxBoundCol) {
        context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIdxCol);
    }
    return idxBoundCol;
}

bool Drawable::bindPosNorUVOpaque()
{
    if(posNorUVOpaqueBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVOpaque);
    }
    return posNorUVOpaqueBound;
}

bool Drawable::bindPosNorUVTransparent()
{
    if(posNorUVTransparentBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorUVTransparent);
    }
    return posNorUVTransparentBound;
}

bool Drawable::bindPosNorCol()
{
    if(posNorColBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPosNorCol);
    }
    return posNorColBound;
}

bool Drawable::bindPos()
{
    if(posBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufPos);
    }
    return posBound;
}

bool Drawable::bindNor()
{
    if(norBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufNor);
    }
    return norBound;
}

bool Drawable::bindUV()
{
    if(uvBound){
        context->glBindBuffer(GL_ARRAY_BUFFER, bufUV);
    }
    return uvBound;
}

