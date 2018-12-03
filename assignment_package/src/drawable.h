#pragma once

#include <openglcontext.h>
#include <la.h>

//This defines a class which can be rendered by our shader program.
//Make any geometry a subclass of ShaderProgram::Drawable in order to render it with the ShaderProgram class.
class Drawable
{
protected:
    int count;     // The number of indices stored in bufIdx.
    int countT;     // The number of indices stored in bufIdx transparent.
    GLuint bufIdx; // A Vertex Buffer Object that we will use to store triangle indices (GLuints)
    GLuint bufIdxT; // A Vertex Buffer Object that we will use to store triangle indices (GLuints) transparent
    GLuint bufIdxCol; // A VBO that we will use for triangle indices for PosNorCol VBOs
    GLuint bufPosNorUVOpaque; // A Vertex Buffer Object that we will use to store mesh vertices, normals, and uvs for opaque blocks
    GLuint bufPosNorUVTransparent; // A Vertex Buffer Object that we will use to store mesh vertices, normals, and uvs for transparent blocks
    GLuint bufPosNorCol;

    bool idxBound; // Set to TRUE by generateIdx(), returned by bindIdx().
    bool idxBoundT;
    bool idxBoundCol;
    bool posNorColBound;
    bool posNorUVOpaqueBound;
    bool posNorUVTransparentBound;

    OpenGLContext* context; // Since Qt's OpenGL support is done through classes like QOpenGLFunctions_3_2_Core,
                          // we need to pass our OpenGL context to the Drawable in order to call GL functions
                          // from within this class.


public:
    Drawable(OpenGLContext* context);
    virtual ~Drawable();

    virtual void create() = 0; // To be implemented by subclasses. Populates the VBOs of the Drawable.
    void destroy(); // Frees the VBOs of the Drawable.

    // Getter functions for various GL data
    virtual GLenum drawMode();
    int elemCount();
    int elemCountT();

    // Call these functions when you want to call glGenBuffers on the buffers stored in the Drawable
    // These will properly set the values of idxBound etc. which need to be checked in ShaderProgram::draw()
    void generateIdx();
    void generateIdxT();
    void generateIdxPosNorCol();
    void generatePosNorCol();
    void generatePosNorUVOpaque();
    void generatePosNorUVTransparent();

    bool bindIdx();
    bool bindIdxT();
    bool bindIdxCol();
    bool bindPosNorCol();
    bool bindPosNorUVOpaque();
    bool bindPosNorUVTransparent();
};
