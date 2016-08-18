#ifndef UTIL_SELECTARGS
#define UTIL_SELECTARGS

namespace util
{

template<typename... tArgs>
struct SelectArgs
{
    template<typename tClass, typename tResult>
    static auto from( tResult (tClass::*pmf)(tArgs...) ) -> decltype(pmf)
    {
        return pmf;
    }
};

} // namespace util

#endif // UTIL_SELECTARGS

