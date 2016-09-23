#ifndef GL_ROOT_H
#define GL_ROOT_H

#include "gl/Global.h"

namespace gl
{

class ContextAccessor
{
public:
    virtual ~ContextAccessor() {}
    virtual void makeCurrent() = 0;
    virtual void doneCurrent() = 0;
};

class Root
{
public:
    Root();

    void setContextAccessor(ContextAccessor& aAccessor);
    void clearContextAccessor();
    void makeCurrent();
    void doneCurrent();

    void setFunctions(Global::Functions& aFunctions);
    void clearFunctions();
    Global::Functions& functions();

private:
    ContextAccessor* mContextAccessor;
    Global::Functions* mFunctions;
};

} // namespace gl

#endif // GL_ROOT_H
