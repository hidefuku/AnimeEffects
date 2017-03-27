#include "ctrl/PoseParam.h"

namespace ctrl
{

PoseParam::PoseParam()
    : mode(PoseEditMode_Move)
    , diWeight(0.0f)
    , eiRadius(200.0f)
    , eiPressure(1.0f)
{
}

} // namespace ctrl
