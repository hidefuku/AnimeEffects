#ifndef IMG_PIXELPOS
#define IMG_PIXELPOS

#include "XC.h"

namespace img
{

struct PixelPos
{
    uint16 x;
    uint16 y;
    uint32 id;
    void set(uint16 aX, uint16 aY) { x = aX; y = aY; }

    void setCenter(const PixelPos& aPos0, const PixelPos& aPos1)
    {
        x = (uint16)(((int)aPos0.x + aPos1.x) / 2);
        y = (uint16)(((int)aPos0.y + aPos1.y) / 2);
    }

    bool tryMergeId(const PixelPos& aRhs)
    {
        if (x == aRhs.x && y == aRhs.y)
        {
            id = aRhs.id;
            return true;
        }
        return false;
    }
};

} // namespace img

#endif // IMG_PIXELPOS

