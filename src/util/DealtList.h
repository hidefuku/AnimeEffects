#ifndef UTIL_DEALTLIST_H
#define UTIL_DEALTLIST_H

#include "XC.h"

namespace util
{

template <typename tObj>
class DealtList;

//-----------------------------------------------------------------
template <typename tObj>
class DealtListNode
{
public:
    DealtListNode()
        : obj()
        , mPrev(0)
        , mNext(0)
	{
	}
    tObj obj;

    DealtListNode<tObj>* prev() const { return mPrev; }
    DealtListNode<tObj>* next() const { return mNext; }

private:
    friend class DealtList<tObj>;

    DealtListNode<tObj>* mPrev;
    DealtListNode<tObj>* mNext;
};

//-----------------------------------------------------------------
// The list which doesn't allocate any nodes.
template <typename tObj>
class DealtList
{
public:
    typedef DealtListNode<tObj> NodeType;

	class Iterator
	{
	public:
		Iterator()
			: mNext(0)
		{
		}

		Iterator(NodeType& aFirst)
			: mNext(&aFirst)
		{
		}

		bool hasNext()
		{
			return mNext != 0 ? true : false; 
		}

        tObj next()
		{
            XC_PTR_ASSERT(mNext);
			NodeType* now = mNext;
            mNext = now->mNext;
            return now->obj;
		}

	private:
		NodeType* mNext;
	};

    DealtList()
		: mFirst(0)
		, mLast(0)
	{
	}

    ~DealtList()
	{
		clear();
	}

	void clear()
	{
		NodeType* next = mFirst;
		while (next) {
			NodeType* now = next;
            next = now->mNext;
            now->mPrev = 0;
            now->mNext = 0;
		}
		mFirst = 0;
		mLast = 0;
	}

	void pushFront(NodeType& aNode)
	{
        aNode.mPrev = 0;
        aNode.mNext = mFirst;
        if (mFirst) mFirst->mPrev = &aNode;
		if (!mLast) mLast = &aNode;
		mFirst = &aNode;
	}

	void pushBack(NodeType& aNode)
	{
        aNode.mPrev = mLast;
        aNode.mNext = 0;
        if (mLast) mLast->mNext = &aNode;
		if (!mFirst) mFirst = &aNode;
		mLast = &aNode;
	}

	void remove(NodeType& aNode)
	{
        if (aNode.mPrev) (*aNode.mPrev).mNext = aNode.mNext;
        if (aNode.mNext) (*aNode.mNext).mPrev = aNode.mPrev;
        if (mFirst == &aNode) mFirst = aNode.mNext;
        if (mLast == &aNode) mLast = aNode.mPrev;
        aNode.mPrev = 0;
        aNode.mNext = 0;
	}

    Iterator iterator()
	{
		return Iterator(*mFirst);
	}

private:
	NodeType* mFirst;
	NodeType* mLast;
};

} // namespace util

#endif // UTIL_DEALTLIST_H
