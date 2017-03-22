#include "ctrl/PoseParam.h"

namespace ctrl
{

PoseParam::PoseParam()
    : mode(PoseEditMode_Move)
    , diRadius(50.0f)
    , diPressure(1.0f)
    , eiRadius(50.0f)
    , eiPressure(1.0f)
{
}

} // namespace ctrl
