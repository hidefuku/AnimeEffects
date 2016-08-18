#ifndef CTRL_BONEPARAM
#define CTRL_BONEPARAM

#include "ctrl/BoneEditMode.h"

namespace ctrl
{

class BoneParam
{
public:
    BoneParam()
        : mode(BoneEditMode_Create)
        , piRadius(50.0f)
        , piPressure(1.0f)
        , eiRadius(50.0f)
        , eiPressure(1.0f)
    {}
    BoneEditMode mode;

    // for paint influence mode
    float piRadius;
    float piPressure;
    // for erase influence mode
    float eiRadius;
    float eiPressure;

};

} // namespace ctrl

#endif // BONEPARAM

