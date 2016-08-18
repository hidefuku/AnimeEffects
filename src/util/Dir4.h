#ifndef UTIL_DIR4
#define UTIL_DIR4

#include "XC.h"

namespace util
{

enum Dir4
{
    Dir4_Up,
    Dir4_Down,
    Dir4_Left,
    Dir4_Right,
    Dir4_TERM
};

util::Dir4 rotateDir(Dir4 aDir, bool aClockwise);
util::Dir4 getInversedDir(Dir4 aDir);

} // namespace util

#endif // UTIL_DIR4

