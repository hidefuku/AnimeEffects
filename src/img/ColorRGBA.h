#ifndef IMG_COLORRGBA
#define IMG_COLORRGBA

#include <QColor>
#include "XC.h"

namespace img
{

struct ColorRGBA
{
    void set(const QColor& aColor)
    {
        v[0] = (uint8)aColor.red();
        v[1] = (uint8)aColor.green();
        v[2] = (uint8)aColor.blue();
        v[3] = (uint8)aColor.alpha();
    }

    void setRGB(const ColorRGBA& aRhs)
    {
        v[0] = aRhs.v[0];
        v[1] = aRhs.v[1];
        v[2] = aRhs.v[2];
    }

    uint8& r() { return v[0]; }
    uint8& g() { return v[1]; }
    uint8& b() { return v[2]; }
    uint8& a() { return v[3]; }

    const uint8& r() const { return v[0]; }
    const uint8& g() const { return v[1]; }
    const uint8& b() const { return v[2]; }
    const uint8& a() const { return v[3]; }

    uint8 v[4];
};

} // namespace img

#endif // IMG_COLORRGBA

