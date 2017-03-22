#ifndef CTRL_POSEPARAM_H
#define CTRL_POSEPARAM_H

#include "ctrl/PoseEditMode.h"

namespace ctrl
{

class PoseParam
{
public:
    PoseParam();

    PoseEditMode mode;

    // for drawing mode
    float diRadius;
    float diPressure;
    // for eraser mode
    float eiRadius;
    float eiPressure;

};

} // namespace ctrl

#endif // CTRL_POSEPARAM_H

