#ifndef UTIL_TREESEEKITERATOR
#define UTIL_TREESEEKITERATOR

#include <vector>
#include "util/ITreeSeeker.h"

namespace util
{

template <typename tData>
class TreeSeekIterator
{
public:
    typedef ITreeSeeker<tData> SeekerType;
    typedef typename SeekerType::Data DataType;
    typedef typename SeekerType::Position PositionType;

    TreeSeekIterator(SeekerType& aSeeker, PositionType aRoot)
        : mSeeker(aSeeker)
        , mPositions()
    {
        if (aRoot)
        {
            mPositions.push_back(aRoot);
        }
    }

    bool hasNext() const
    {
        return !mPositions.empty();
    }

    PositionType next()
    {
        XC_PTR_ASSERT(!mPositions.empty());
        PositionType next = mPositions.back();
        toNext();
        return next;
    }

    DataType data(PositionType aPos)
    {
        return mSeeker.data(aPos);
    }

private:
    void toNext()
    {
        XC_ASSERT(!mPositions.empty());

        auto current = mPositions.back();
        auto child = mSeeker.child(current);
        if (child)
        {
            mPositions.push_back(child);
            return;
        }

        while (!mPositions.empty())
        {
            auto next = mSeeker.nextSib(mPositions.back());
            if (next)
            {
                mPositions.back() = next;
                return;
            }

            mPositions.pop_back();
        }
    }

    SeekerType& mSeeker;
    std::vector<PositionType> mPositions;
};

} // namespace util

#endif // UTIL_TREESEEKITERATOR

