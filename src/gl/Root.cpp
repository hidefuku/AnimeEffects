#include "XC.h"
#include "gl/Root.h"

namespace gl
{

Root::Root()
    : mContextAccessor()
    , mFunctions()
{
}

void Root::setContextAccessor(ContextAccessor& aAccessor)
{
    mContextAccessor = &aAccessor;
}

void Root::clearContextAccessor()
{
    mContextAccessor = nullptr;
}

void Root::makeCurrent()
{
    XC_PTR_ASSERT(mContextAccessor);
    mContextAccessor->makeCurrent();
}

void Root::doneCurrent()
{
    XC_PTR_ASSERT(mContextAccessor);
    mContextAccessor->doneCurrent();
}

void Root::setFunctions(Global::Functions& aFunctions)
{
    XC_ASSERT(!mFunctions);
    mFunctions = &aFunctions;
}

void Root::clearFunctions()
{
    mFunctions = nullptr;
}

Global::Functions& Root::functions()
{
    XC_PTR_ASSERT(mFunctions);
    return *mFunctions;
}

} // namespace gl
