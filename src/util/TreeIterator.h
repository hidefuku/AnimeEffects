#ifndef UTIL_TREEITERATOR_H
#define UTIL_TREEITERATOR_H

#include "XC.h"

namespace util
{

template<typename tTree, typename tChildrenIterator>
class TreeIterator
{
    struct Layer
    {
        Layer(tTree* aParent) : parent(aParent), itr(aParent->children().begin()) {}
        bool isEnd() const { return itr == parent->children().end(); }
        void forward() { ++itr; }
        Layer dived() const { return Layer(*itr); }
        bool canDive() const { return !((*itr)->children().empty()); }
        tTree* ptr() const { return *itr; }
        tTree* parent;
        tChildrenIterator itr;
    };
    std::vector<Layer> mLayers;
    tTree* mRoot;
    tTree* mNext;

    void toNext()
    {
        mNext = NULL;

        // initialize
        if (mLayers.empty())
        {
            mLayers.push_back(Layer(mRoot));
            if (!mLayers.back().isEnd())
            {
                mNext = mLayers.back().ptr();
            }
            return;
        }

        // dive
        if (mLayers.back().canDive())
        {
            mLayers.push_back(mLayers.back().dived());
            mNext = mLayers.back().ptr();
            return;
        }
        else
        {
            while (1)
            {
                // forward
                mLayers.back().forward();
                if (!mLayers.back().isEnd())
                {
                    mNext = mLayers.back().ptr();
                    return;
                }

                // rise
                mLayers.pop_back();
                if (mLayers.empty())
                {
                    return;
                }
            }
        }
    }

public:
    TreeIterator(tTree* aRoot)
        : mLayers()
        , mRoot(aRoot)
        , mNext(aRoot)
    {
    }

    bool hasNext() const
    {
        return mNext;
    }

    tTree* next()
    {
        XC_PTR_ASSERT(mNext);
        tTree* next = mNext;
        toNext();
        return next;
    }
};

} // namespace util

#endif // UTIL_TREEITERATOR_H
