#ifndef UTIL_IDSOLVER
#define UTIL_IDSOLVER

#include <functional>
#include <QMap>

namespace util
{

template<typename tData>
class IDSolver
{
public:
    typedef int IdType;
    typedef std::function<void(tData)> Solver;
    typedef std::pair<IdType, Solver> Referencer;

    IDSolver()
        : mDataMap()
        , mReferencers()
    {
    }

    void pushData(IdType aId, tData aData)
    {
        mDataMap[aId] = aData;
    }

    void pushReferencer(IdType aId, const Solver& aSolver)
    {
        mReferencers.push_back(Referencer(aId, aSolver));
    }

    // we guarantee to call each solvers by pushing order.
    bool solve()
    {
        for (auto refer : mReferencers)
        {
            if (!mDataMap.contains(refer.first))
            {
                return false;
            }
            refer.second(mDataMap[refer.first]);
        }
        return true;
    }

private:
    QMap<IdType, tData> mDataMap;
    QVector<Referencer> mReferencers;
};

} // namespace util

#endif // UTIL_IDSOLVER

