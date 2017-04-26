#include "TextUtil.h"
#include "XC.h"

namespace util
{

//-------------------------------------------------------------------------------------------------
TextUtil::ShiftJisDecoder::ShiftJisDecoder()
{
    mCodec = QTextCodec::codecForName("Shift-JIS");
    mDecoder = mCodec->makeDecoder();
}

TextUtil::ShiftJisDecoder::~ShiftJisDecoder()
{
    delete mDecoder;
}

QString TextUtil::ShiftJisDecoder::decode(const char* aText) const
{
    return mDecoder->toUnicode(aText);
}

//-------------------------------------------------------------------------------------------------
float TextUtil::getShiftJisScore(const char* aStr, size_t aSize)
{
    static const float kFailureScore = -1.0f;

    if (aStr == NULL || aSize <= 0) return kFailureScore;

    const unsigned char* p = (const unsigned char*)aStr;
    long long scoreSum = 0;
    int prev = -1;
    int count = 0;

    for (size_t i = 0; i < aSize; ++i, ++p, ++count)
    {
        unsigned char c1 = *p;
        if (c1 == 0x00)
        {
            return kFailureScore;
        }
        else if (c1 <= 0x7f)
        {
            // ascii
            scoreSum += 50;
            prev = 0;
        }
        else if (0xa1 <= c1 && c1 <= 0xdf)
        {
            // 1byte kana
            if      (prev == 0) scoreSum += 49;
            else if (prev == 1) scoreSum += 51;
            else if (prev == 4) scoreSum += 40;
            else if (prev == 6) scoreSum += 40;
            else                scoreSum += 50;
            prev = 1;
        }
        else if ((0x81 <= c1 && c1 <= 0x9f) || (0xe0 <= c1 && c1 <= 0xfc))
        {
            if (i == aSize - 1) return kFailureScore;

            // 2byte code
            ++i;
            ++p;
            unsigned char c2 = *p;
            uint16 c12 = (((uint16)c1) << 8) | ((uint16)c2);
            if ((0x40 <= c2 && c2 <= 0x7e) || (0x80 <= c2 && c2 <= 0xfc))
            {
                if (c12 <= 0x824e)
                {
                    // kigou1
                    if      (prev == 2) scoreSum += 65;
                    else                scoreSum += 60;
                    prev = 2;
                }
                else if (c12 <= 0x8396)
                {
                    // number + kana
                    if      (prev == 1) scoreSum += 51;
                    else if (prev == 3) scoreSum += 65;
                    else if (prev == 4) scoreSum += 55;
                    else                scoreSum += 60;
                    prev = 3;
                }
                else if (c12 <= 0x889e)
                {
                    // kigou2
                    if      (prev == 0) scoreSum += 45;
                    else if (prev == 1) scoreSum += 45;
                    else if (prev == 3) scoreSum += 45;
                    else if (prev == 4) scoreSum += 55;
                    else if (prev == 6) scoreSum += 40;
                    else                scoreSum += 49;
                    prev = 4;
                }
                else if (c1 <= 0x9f)
                {
                    // kanji1
                    if      (prev == 0) scoreSum += 55;
                    else if (prev == 1) scoreSum += 50;
                    else if (prev == 3) scoreSum += 65;
                    else if (prev == 4) scoreSum += 40;
                    else if (prev == 5) scoreSum += 65;
                    else                scoreSum += 60;
                    prev = 5;
                }
                else if (c12 < 0xeaa5)
                {
                    // kanji2
                    if      (prev == 0) scoreSum += 45;
                    else if (prev == 1) scoreSum += 45;
                    else if (prev == 4) scoreSum += 40;
                    else                scoreSum += 50;
                    prev = 6;
                }
                else
                {
                    return kFailureScore;
                }
            }
            else
            {
                return kFailureScore;
            }

        }
        else
        {
            return kFailureScore;
        }
    }
    return std::min(1.0f, 0.01f + 0.01f * (float)(scoreSum / count));
}

//-------------------------------------------------------------------------------------------------
QStringList TextUtil::splitAndTrim(const QString& aText, QChar aSplit)
{
    if (aText.trimmed().isEmpty())
    {
        return QStringList();
    }

    QStringList texts = aText.split(aSplit);
    for (QString& text : texts)
    {
        text = text.trimmed();
    }
    return texts;
}

} // namespace util
