#pragma once

#include <openglcontext.h>
#include <la.h>
#include <memory>

class Texture
{
public:
    Texture(OpenGLContext* context);
    ~Texture();

    void create(const char *texturePath);
    void load(int texSlot);
    void bind(int texSlot);
    std::shared_ptr<QImage> m_textureImage;

private:
    OpenGLContext* context;
    GLuint m_textureHandle;
};
