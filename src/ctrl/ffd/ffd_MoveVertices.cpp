#include "cmnd/BasicCommands.h"
#include "ctrl/ffd/ffd_MoveVertices.h"

namespace ctrl {
namespace ffd {

MoveVertices::MoveVertices()
    : mFixed(false)
{
}

void MoveVertices::push(cmnd::AssignMemory* aCommand)
{
    XC_ASSERT(!mFixed);
    commands().push(aCommand);
}

cmnd::AssignMemory* MoveVertices::assign(int aIndex)
{
    return (cmnd::AssignMemory*)commands().at(aIndex);
}

} // namespace ffd
} // namespace ctrl
