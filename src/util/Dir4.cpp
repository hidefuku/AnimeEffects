#include "util/Dir4.h"

namespace util
{

util::Dir4 rotateDir(Dir4 aDir, bool aClockwise)
{
    if (aClockwise)
    {
        switch (aDir)
        {
        case Dir4_Up: return Dir4_Right;
        case Dir4_Down: return Dir4_Left;
        case Dir4_Left: return Dir4_Up;
        case Dir4_Right: return Dir4_Down;
        default: return Dir4_Up;
        }
    }
    else
    {
        switch (aDir)
        {
        case Dir4_Up: return Dir4_Left;
        case Dir4_Down: return Dir4_Right;
        case Dir4_Left: return Dir4_Down;
        case Dir4_Right: return Dir4_Up;
        default: return Dir4_Up;
        }
    }
    XC_ASSERT(0);
    return Dir4_Up;
}

util::Dir4 getInversedDir(Dir4 aDir)
{
    switch (aDir)
    {
    case Dir4_Up: return Dir4_Down;
    case Dir4_Down: return Dir4_Up;
    case Dir4_Left: return Dir4_Right;
    case Dir4_Right: return Dir4_Left;
    default: return Dir4_Up;
    }
    XC_ASSERT(0);
    return Dir4_Up;
}

} // namespace util
