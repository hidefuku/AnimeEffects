#ifndef UTIL_DEALTLIST_H
#define UTIL_DEALTLIST_H

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
        , prev(0)
        , next(0)
	{
	}
    tObj obj;
    DealtListNode<tObj>* prev;
    DealtListNode<tObj>* next;
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
			VISCUM_ASSERT(mNext);
			NodeType* now = mNext;
            mNext = now->next;
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
            next = now->next;
            now->prev = 0;
            now->next = 0;
		}
		mFirst = 0;
		mLast = 0;
	}

	void pushFront(NodeType& aNode)
	{
        aNode.prev = 0;
        aNode.next = mFirst;
        if (mFirst) mFirst->prev = &aNode;
		if (!mLast) mLast = &aNode;
		mFirst = &aNode;
	}

	void pushBack(NodeType& aNode)
	{
        aNode.prev = mLast;
        aNode.next = 0;
        if (mLast) mLast->next = &aNode;
		if (!mFirst) mFirst = &aNode;
		mLast = &aNode;
	}

	void remove(NodeType& aNode)
	{
        if (aNode.prev) (*aNode.prev).mNext = aNode.next;
        if (aNode.next) (*aNode.next).mPrev = aNode.prev;
        if (mFirst == &aNode) mFirst = aNode.next;
        if (mLast == &aNode) mLast = aNode.prev;
        aNode.prev = 0;
        aNode.next = 0;
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
