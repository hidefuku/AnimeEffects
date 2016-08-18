#ifndef CMND_STABLE
#define CMND_STABLE

#include "cmnd/Base.h"

namespace cmnd
{

class Stable : public Base
{
public:
    Stable() {}
    virtual ~Stable() {}

    virtual void exec() { redo(); }
    virtual void redo() {}
    virtual void undo() {}

private:
    virtual bool tryExec() final { exec(); return true; }
    virtual bool tryRedo() final { redo(); return true; }
    virtual bool tryUndo() final { undo(); return true; }
};

} // namespace cmnd

#endif // CMND_STABLE

