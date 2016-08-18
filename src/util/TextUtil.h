#ifndef UTIL_TEXTUTIL_H
#define UTIL_TEXTUTIL_H

#include <QTextCodec>
#include <QTextDecoder>

namespace util
{

class TextUtil
{
public:
    class ShiftJisDecoder
    {
    public:
        ShiftJisDecoder();
        ~ShiftJisDecoder();
        QString decode(const char* aText) const;

    private:
        QTextCodec* mCodec;
        QTextDecoder* mDecoder;
    };

    static float getShiftJisScore(const char* aStr, size_t aSize);

private:
    TextUtil() {}
};

} // namespace util

#endif // UTIL_TEXTUTIL_H
