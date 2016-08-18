#ifndef UTIL_NONCOPYABLE
#define UTIL_NONCOPYABLE

namespace util
{

class NonCopyable
{
protected:
    NonCopyable() {}
    ~NonCopyable() {}

private:
    NonCopyable(const NonCopyable&);
    NonCopyable& operator=(const NonCopyable&);
};

} // namespace util

#endif // UTIl_NONCOPYABLE

