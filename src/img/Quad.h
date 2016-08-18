#ifndef IMG_QUAD
#define IMG_QUAD

#include "img/PixelPos.h"

namespace img
{

struct Quad
{
    PixelPos pos[4]; // left top, right top, left bottom, right bottom

    inline PixelPos& lt() { return pos[0]; }
    inline PixelPos& rt() { return pos[1]; }
    inline PixelPos& lb() { return pos[2]; }
    inline PixelPos& rb() { return pos[3]; }
    inline const PixelPos& lt() const { return pos[0]; }
    inline const PixelPos& rt() const { return pos[1]; }
    inline const PixelPos& lb() const { return pos[2]; }
    inline const PixelPos& rb() const { return pos[3]; }

    inline bool hasLRange() const { return lt().id != lb().id; }
    inline bool hasTRange() const { return lt().id != rt().id; }
    inline bool hasRRange() const { return rt().id != rb().id; }
    inline bool hasBRange() const { return lb().id != rb().id; }

    inline void reset()
    {
        pos[0].set(0xffff, 0xffff);
        pos[1].set(0, 0xffff);
        pos[2].set(0xffff, 0);
        pos[3].set(0, 0);
    }

    inline void extend(uint16 aX, uint16 aY)
    {
        if (aX <  pos[0].x) { pos[0].x = pos[2].x = aX;}
        if (pos[3].x <= aX) { pos[1].x = pos[3].x = aX + 1;}
        if (aY <  pos[0].y) { pos[0].y = pos[1].y = aY;}
        if (pos[3].y <= aY) { pos[2].y = pos[3].y = aY + 1;}
    }

    inline void makeSureConvex()
    {
        if (!isConvex(pos[0], pos[1], pos[3]))
        {
            pos[1].setCenter(pos[0], pos[3]);
        }
        if (!isConvex(pos[1], pos[3], pos[2]))
        {
            pos[3].setCenter(pos[1], pos[2]);
        }
        if (!isConvex(pos[3], pos[2], pos[0]))
        {
            pos[2].setCenter(pos[3], pos[0]);
        }
        if (!isConvex(pos[2], pos[0], pos[1]))
        {
            pos[0].setCenter(pos[2], pos[1]);
        }
    }

private:
    inline bool isConvex(const PixelPos& aPos0, const PixelPos& aPos1, const PixelPos& aPos2) const
    {
        int ax = (int)aPos2.x - aPos0.x;
        int ay = (int)aPos2.y - aPos0.y;
        int bx = (int)aPos1.x - aPos0.x;
        int by = (int)aPos1.y - aPos0.y;
        return (ax * by - ay * bx) <= 0;
    }
};

} // namespace img

#endif // IMG_QUAD

