#include "shaderprogram.h"
#include <QFile>
#include <QStringBuilder>
#include <QTextStream>
#include <QDebug>
#include <iostream>


ShaderProgram::ShaderProgram(OpenGLContext *context)
    : vertShader(), fragShader(), prog(),
      attrPos(-1), attrNor(-1), attrCol(-1), attrUV(-1), attrCos(-1), attrAnimate(-1),
      unifModel(-1), unifModelInvTr(-1), unifViewProj(-1), unifColor(-1),
      unifSampler2D(-1), unifTime(-1), unifDimensions(-1), unifView(-1), unifPlayer(-1), unifMode(-1),
      context(context)
{}

void ShaderProgram::create(const char *vertfile, const char *fragfile)
{
    // Allocate space on our GPU for a vertex shader and a fragment shader and a shader program to manage the two
    vertShader = context->glCreateShader(GL_VERTEX_SHADER);
    fragShader = context->glCreateShader(GL_FRAGMENT_SHADER);
    prog = context->glCreateProgram();
    // Get the body of text stored in our two .glsl files
    QString qVertSource = qTextFileRead(vertfile);
    QString qFragSource = qTextFileRead(fragfile);

    char* vertSource = new char[qVertSource.size()+1];
    strcpy(vertSource, qVertSource.toStdString().c_str());
    char* fragSource = new char[qFragSource.size()+1];
    strcpy(fragSource, qFragSource.toStdString().c_str());


    // Send the shader text to OpenGL and store it in the shaders specified by the handles vertShader and fragShader
    context->glShaderSource(vertShader, 1, &vertSource, 0);
    context->glShaderSource(fragShader, 1, &fragSource, 0);
    // Tell OpenGL to compile the shader text stored above
    context->glCompileShader(vertShader);
    context->glCompileShader(fragShader);
    // Check if everything compiled OK
    GLint compiled;
    context->glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(vertShader);
    }
    context->glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        printShaderInfoLog(fragShader);
    }

    // Tell prog that it manages these particular vertex and fragment shaders
    context->glAttachShader(prog, vertShader);
    context->glAttachShader(prog, fragShader);
    context->glLinkProgram(prog);

    // Check for linking success
    GLint linked;
    context->glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        printLinkInfoLog(prog);
    }

    // Get the handles to the variables stored in our shaders
    // See shaderprogram.h for more information about these variables

    attrPos = context->glGetAttribLocation(prog, "vs_Pos");
    attrNor = context->glGetAttribLocation(prog, "vs_Nor");
    attrCol = context->glGetAttribLocation(prog, "vs_Col");
    attrUV  = context->glGetAttribLocation(prog, "vs_UV");
    attrCos = context->glGetAttribLocation(prog, "vs_Cos");
    attrAnimate = context->glGetAttribLocation(prog, "vs_Animate");

    unifModel      = context->glGetUniformLocation(prog, "u_Model");
    unifModelInvTr = context->glGetUniformLocation(prog, "u_ModelInvTr");
    unifViewProj   = context->glGetUniformLocation(prog, "u_ViewProj");
    unifColor      = context->glGetUniformLocation(prog, "u_Color");
    unifDimensions = context->glGetUniformLocation(prog, "u_Dimensions");
    unifTime = context->glGetUniformLocation(prog, "u_Time");
    unifSampler2D = context->glGetUniformLocation(prog, "u_RenderedTexture");
    unifView = context->glGetUniformLocation(prog, "u_ViewVec");
    unifPlayer = context->glGetUniformLocation(prog, "u_Player");
    unifMode = context->glGetUniformLocation(prog, "u_Mode");
}

void ShaderProgram::useMe()
{
    context->glUseProgram(prog);
}

void ShaderProgram::setModelMatrix(const glm::mat4 &model)
{
    useMe();

    if (unifModel != -1) {
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModel,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &model[0][0]);
    }

    if (unifModelInvTr != -1) {
        glm::mat4 modelinvtr = glm::inverse(glm::transpose(model));
        // Pass a 4x4 matrix into a uniform variable in our shader
                        // Handle to the matrix variable on the GPU
        context->glUniformMatrix4fv(unifModelInvTr,
                        // How many matrices to pass
                           1,
                        // Transpose the matrix? OpenGL uses column-major, so no.
                           GL_FALSE,
                        // Pointer to the first element of the matrix
                           &modelinvtr[0][0]);
    }
}

void ShaderProgram::setViewProjMatrix(const glm::mat4 &vp)
{
    // Tell OpenGL to use this shader program for subsequent function calls
    useMe();

    if(unifViewProj != -1) {
    // Pass a 4x4 matrix into a uniform variable in our shader
                    // Handle to the matrix variable on the GPU
    context->glUniformMatrix4fv(unifViewProj,
                    // How many matrices to pass
                       1,
                    // Transpose the matrix? OpenGL uses column-major, so no.
                       GL_FALSE,
                    // Pointer to the first element of the matrix
                       &vp[0][0]);
    }
}

void ShaderProgram::setGeometryColor(glm::vec4 color)
{
    useMe();

    if(unifColor != -1)
    {
        context->glUniform4fv(unifColor, 1, &color[0]);
    }
}

//This function, as its name implies, uses the passed in GL widget
void ShaderProgram::draw(Drawable &d, int texSlot = 0)
{
        useMe();
        if(unifSampler2D != -1)
        {
            context->glUniform1i(unifSampler2D, texSlot);
        }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

        // Remember, by calling bindPos(), we call
        // glBindBuffer on the Drawable's VBO for vertex position,
        // meaning that glVertexAttribPointer associates vs_Pos
        // (referred to by attrPos) with that VBO
    if (attrPos != -1 && d.bindPosNorUVOpaque()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), 0);
    }
    if (attrNor != -1 && d.bindPosNorUVOpaque()) {
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(4*sizeof(GL_FLOAT)));
    }
    if (attrUV != -1 && d.bindPosNorUVOpaque()) {
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(8*sizeof(GL_FLOAT)));
    }
    if (attrCos != -1 && d.bindPosNorUVOpaque()) {
        context->glEnableVertexAttribArray(attrCos);
        context->glVertexAttribPointer(attrCos, 1, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(10*sizeof(GL_FLOAT)));
    }
    if (attrAnimate != -1 && d.bindPosNorUVOpaque()) {
        context->glEnableVertexAttribArray(attrAnimate);
        context->glVertexAttribPointer(attrAnimate, 1, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(11*sizeof(GL_FLOAT)));
    }

        d.bindIdx();
        context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);
    if (attrCos != -1) context->glDisableVertexAttribArray(attrCos);
    if (attrAnimate != -1) context->glDisableVertexAttribArray(attrAnimate);

    context->printGLErrorLog();
}

void ShaderProgram::drawT(Drawable &d)
{
        useMe();
        if(unifSampler2D != -1)
        {
            context->glUniform1i(unifSampler2D, 0);
        }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

        // Remember, by calling bindPos(), we call
        // glBindBuffer on the Drawable's VBO for vertex position,
        // meaning that glVertexAttribPointer associates vs_Pos
        // (referred to by attrPos) with that VBO
        if (attrPos != -1 && d.bindPosNorUVTransparent()) {
            context->glEnableVertexAttribArray(attrPos);
            context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), 0);
        }
        if (attrNor != -1 && d.bindPosNorUVTransparent()) {
            context->glEnableVertexAttribArray(attrNor);
            context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(4*sizeof(GL_FLOAT)));
        }
        if (attrUV != -1 && d.bindPosNorUVTransparent()) {
            context->glEnableVertexAttribArray(attrUV);
            context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(8*sizeof(GL_FLOAT)));
        }
        if (attrCos != -1 && d.bindPosNorUVTransparent()) {
            context->glEnableVertexAttribArray(attrCos);
            context->glVertexAttribPointer(attrCos, 1, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(10*sizeof(GL_FLOAT)));
        }
        if (attrAnimate != -1 && d.bindPosNorUVTransparent()) {
            context->glEnableVertexAttribArray(attrAnimate);
            context->glVertexAttribPointer(attrAnimate, 1, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(11*sizeof(GL_FLOAT)));
        }

        // Bind the index buffer and then draw shapes from it.
        // This invokes the shader program, which accesses the vertex buffers.
        d.bindIdxT();
        context->glDrawElements(d.drawMode(), d.elemCountT(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);
    if (attrCos != -1) context->glDisableVertexAttribArray(attrCos);
    if (attrAnimate != -1) context->glDisableVertexAttribArray(attrAnimate);

    context->printGLErrorLog();
}

void ShaderProgram::drawPosNorCol(Drawable &d)
{
        useMe();
        if(unifSampler2D != -1)
        {
            context->glUniform1i(unifSampler2D, 0);
        }

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

        // Remember, by calling bindPos(), we call
        // glBindBuffer on the Drawable's VBO for vertex position,
        // meaning that glVertexAttribPointer associates vs_Pos
        // (referred to by attrPos) with that VBO
    if (attrPos != -1 && d.bindPosNorCol()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), 0);
    }
    if (attrNor != -1 && d.bindPosNorCol()) {
        context->glEnableVertexAttribArray(attrNor);
        context->glVertexAttribPointer(attrNor, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(4*sizeof(GL_FLOAT)));
    }
    if (attrCol != -1 && d.bindPosNorCol()) {
        context->glEnableVertexAttribArray(attrCol);
        context->glVertexAttribPointer(attrCol, 4, GL_FLOAT, false, 12*sizeof(GL_FLOAT), (GLvoid*)(8*sizeof(GL_FLOAT)));
    }

        d.bindIdxCol();
        context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrNor != -1) context->glDisableVertexAttribArray(attrNor);
    if (attrCol != -1) context->glDisableVertexAttribArray(attrCol);

    context->printGLErrorLog();
}

void ShaderProgram::drawOverlay(Drawable &d)
{
    useMe();

    // Set our "renderedTexture" sampler to user Texture Unit 0
    context->glUniform1i(unifSampler2D, 1);

    // Each of the following blocks checks that:
    //   * This shader has this attribute, and
    //   * This Drawable has a vertex buffer for this attribute.
    // If so, it binds the appropriate buffers to each attribute.

    if (attrPos != -1 && d.bindPos()) {
        context->glEnableVertexAttribArray(attrPos);
        context->glVertexAttribPointer(attrPos, 4, GL_FLOAT, false, 0, NULL);
    }
    if (attrUV != -1 && d.bindUV()) {
        context->glEnableVertexAttribArray(attrUV);
        context->glVertexAttribPointer(attrUV, 2, GL_FLOAT, false, 0, NULL);
    }

    // Bind the index buffer and then draw shapes from it.
    // This invokes the shader program, which accesses the vertex buffers.
    d.bindIdx();
    context->glDrawElements(d.drawMode(), d.elemCount(), GL_UNSIGNED_INT, 0);

    if (attrPos != -1) context->glDisableVertexAttribArray(attrPos);
    if (attrUV != -1) context->glDisableVertexAttribArray(attrUV);

    context->printGLErrorLog();
}

char* ShaderProgram::textFileRead(const char* fileName) {
    char* text;

    if (fileName != NULL) {
        FILE *file = fopen(fileName, "rt");

        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            int count = ftell(file);
            rewind(file);

            if (count > 0) {
                text = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(text, sizeof(char), count, file);
                text[count] = '\0';	//cap off the string with a terminal symbol, fixed by Cory
            }
            fclose(file);
        }
    }
    return text;
}

QString ShaderProgram::qTextFileRead(const char *fileName)
{
    QString text;
    QFile file(fileName);
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        text = in.readAll();
        text.append('\0');
    }
    return text;
}

void ShaderProgram::printShaderInfoLog(int shader)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0)
    {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetShaderInfoLog(shader,infoLogLen, &charsWritten, infoLog);
        qDebug() << "ShaderInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }

    // should additionally check for OpenGL errors here
}

void ShaderProgram::printLinkInfoLog(int prog)
{
    int infoLogLen = 0;
    int charsWritten = 0;
    GLchar *infoLog;

    context->glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLogLen);

    // should additionally check for OpenGL errors here

    if (infoLogLen > 0) {
        infoLog = new GLchar[infoLogLen];
        // error check for fail to allocate memory omitted
        context->glGetProgramInfoLog(prog, infoLogLen, &charsWritten, infoLog);
        qDebug() << "LinkInfoLog:" << endl << infoLog << endl;
        delete [] infoLog;
    }
}

void ShaderProgram::setDimensions(glm::ivec2 dims)
{
    useMe();

    if(unifDimensions != -1)
    {
        context->glUniform2i(unifDimensions, dims.x, dims.y);
    }
}

void ShaderProgram::setTime(int t)
{
    useMe();

    if(unifTime != -1)
    {
        context->glUniform1i(unifTime, t);
    }
}

void ShaderProgram::setUnifMode(int mode)
{
    useMe();

    if(unifMode != -1)
    {
        context->glUniform1i(unifMode, mode);
    }
}

void ShaderProgram::setViewVector(glm::vec4 view) {
    useMe();

    if(unifView != -1)
    {
        context->glUniform4fv(unifView, 1, &view[0]);
    }
}

void ShaderProgram::setPlayerPos(glm::vec4 pos) {
    useMe();

    if(unifPlayer != -1)
    {
        context->glUniform4fv(unifPlayer, 1, &pos[0]);
    }
}
