#ifndef UTIL_ITREESEEKER
#define UTIL_ITREESEEKER

namespace util
{

template <typename tData>
class ITreeSeeker
{
public:
    typedef tData Data;
    typedef void* Position;

    ITreeSeeker() {}
    virtual ~ITreeSeeker() {}

    virtual Data data(Position) const = 0;
    virtual Position parent(Position) const = 0;
    virtual Position child(Position) const = 0;
    virtual Position prevSib(Position) const = 0;
    virtual Position nextSib(Position) const = 0;
};


} // namespace util

#endif // UTIL_ITREESEEKER

