#ifndef UTIL_IDASSIGNER
#define UTIL_IDASSIGNER

#include <QMap>

namespace util
{

template<typename tData>
class IDAssigner
{
public:
    typedef int IdType;

    IDAssigner()
        : mMap()
        , mCurrent(0)
    {
    }

    IdType getId(tData aData)
    {
        if (mMap.contains(aData))
        {
            return mMap[aData];
        }
        else
        {
            auto id = mCurrent;
            ++mCurrent;
            mMap[aData] = id;
            return id;
        }
    }

    void clear()
    {
        mMap.clear();
        mCurrent = 0;
    }

private:
    QMap<tData, IdType> mMap;
    IdType mCurrent;
};

} // namespace util

#endif // UTIL_IDASSIGNER

