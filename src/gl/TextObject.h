#ifndef GL_TEXTOBJECT_H
#define GL_TEXTOBJECT_H

#include <memory>
#include "gl/Texture.h"

namespace gl
{

class TextObject
{
public:
    typedef std::shared_ptr<gl::Texture> WorkCache;

    TextObject();
    explicit TextObject(const QString& aText);

    void setText(const QString& aText);
    const QString& text() const { return mText; }

    quint16 checksum() const { return mCRC16; }

    gl::Texture& texture() { return mTexture; }
    const gl::Texture& texture() const { return mTexture; }

    WorkCache& workCache() { return mWorkCache; }
    const WorkCache& workCache() const { return mWorkCache; }

private:
    QString mText;
    quint16 mCRC16;
    gl::Texture mTexture;
    WorkCache mWorkCache;
};

} // namespace gl

#endif // GL_TEXTOBJECT_H
