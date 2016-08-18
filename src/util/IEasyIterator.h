#ifndef UTIL_IEASYITERATOR
#define UTIL_IEASYITERATOR

namespace util
{

template <typename tObj>
class IEasyIterator
{
public:
    virtual ~IEasyIterator() {}
    virtual bool hasNext() const = 0;
    tObj next() = 0;
};

} // namespace util

#endif // UTIL_IEASYITERATOR

