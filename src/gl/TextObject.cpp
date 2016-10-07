#include "gl/TextObject.h"

namespace gl
{

TextObject::MapKey TextObject::getMapKey(const QString& aText)
{
    auto bytes = aText.toUtf8();
    auto crc16 = qChecksum(bytes.data(), bytes.size());
    return MapKey(crc16, aText);
}

TextObject::TextObject()
    : mText()
    , mCRC16()
    , mTexture()
    , mWorkCache()
{
}

TextObject::TextObject(const QString& aText)
    : mText()
    , mCRC16()
    , mTexture()
    , mWorkCache()
{
    setText(aText);
}

void TextObject::setText(const QString& aText)
{
    mText = aText;
    auto bytes = mText.toUtf8();
    mCRC16 = qChecksum(bytes.data(), bytes.size());
}

int TextObject::pixelCount() const
{
    return mTexture.size().width() * mTexture.size().height();
}

} // namespace gl
