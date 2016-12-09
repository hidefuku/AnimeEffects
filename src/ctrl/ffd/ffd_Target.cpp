#include "gl/Global.h"
#include "ctrl/ffd/ffd_Target.h"

namespace ctrl {
namespace ffd {

//-------------------------------------------------------------------------------------------------
Target::Target()
    : node()
    , keyOwner()
    , task()
{
}

Target::Target(core::ObjectNode* aNode)
    : node(aNode)
    , keyOwner()
    , task()
{
}

Target::~Target()
{
    keyOwner.deleteOwnsKey();

    gl::Global::makeCurrent();
    task.reset();
}

bool Target::isValid() const
{
    return (bool)node && (bool)keyOwner;
}

//-------------------------------------------------------------------------------------------------
bool Targets::hasValidTarget() const
{
    for (int i = 0; i < this->size(); ++i)
    {
        if ((*this)[i]->isValid()) return true;
    }
    return false;
}


} // namespace ffd
} // namespace ctrl
