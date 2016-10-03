#include "gl/TextObject.h"

namespace gl
{

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

} // namespace gl
