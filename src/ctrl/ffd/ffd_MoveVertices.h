#ifndef CTRL_FFD_MOVEVERTICES_H
#define CTRL_FFD_MOVEVERTICES_H

#include "cmnd/Scalable.h"
namespace cmnd { class AssignMemory; }

namespace ctrl {
namespace ffd {

class MoveVertices : public cmnd::Scalable
{
public:
    MoveVertices();
    void push(cmnd::AssignMemory* aCommand);
    cmnd::AssignMemory* assign(int aIndex);

private:
    virtual void initialize() { mFixed = true; }
    bool mFixed;
};

} // namespace ffd
} // namespace ctrl

#endif // CTRL_FFD_MOVEVERTICES_H
