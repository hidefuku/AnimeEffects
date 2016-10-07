#ifndef GL_TEXTOBJECT_H
#define GL_TEXTOBJECT_H

#include <memory>
#include "gl/Texture.h"

namespace gl
{

class TextObject
{
public:
    struct MapKey
    {
        quint16 checksum;
        QString text;

        MapKey()
            : checksum(), text() {}
        MapKey(quint16 aChecksum, const QString& aText)
            : checksum(aChecksum), text(aText) {}

        bool operator <(const MapKey& aRhs) const
        {
            if (checksum < aRhs.checksum) { return true; }
            else if (checksum == aRhs.checksum) { return text < aRhs.text; }
            else { return false; }
        }
    };

    typedef std::shared_ptr<gl::Texture> WorkCache;

    static MapKey getMapKey(const QString& aText);

    TextObject();
    explicit TextObject(const QString& aText);

    void setText(const QString& aText);
    const QString& text() const { return mText; }

    quint16 checksum() const { return mCRC16; }
    MapKey mapKey() const { return MapKey(mCRC16, mText); }
    int pixelCount() const;

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
