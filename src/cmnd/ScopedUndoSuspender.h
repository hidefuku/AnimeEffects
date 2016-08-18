#ifndef CMND_SCOPEDUNDOSUSPENDER_H
#define CMND_SCOPEDUNDOSUSPENDER_H

#include "cmnd/Stack.h"

namespace cmnd
{

class ScopedUndoSuspender
{
    Stack& mStack;
public:
    ScopedUndoSuspender(Stack& aStack)
        : mStack(aStack)
    {
        mStack.suspendUndo();
    }

    ~ScopedUndoSuspender()
    {
        mStack.resumeUndo();
    }
};

} // namespace cmnd

#endif // CMND_SCOPEDUNDOSUSPENDER_H
