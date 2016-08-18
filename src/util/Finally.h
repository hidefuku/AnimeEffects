#ifndef UTIL_FINALLY
#define UTIL_FINALLY

#include <functional>

namespace util
{

class Finally
{
    typedef std::function<void()> FunctionType;
    FunctionType mDest;

public:
    Finally(FunctionType aDest)
        : mDest(aDest)
    {
    }

    ~Finally()
    {
        mDest();
    }
};

} // namespace util

#endif // UTIL_FINALLY
