#ifndef UTIL_TREENODEBASE_H
#define UTIL_TREENODEBASE_H

#include "XC.h"

namespace util
{

//-------------------------------------------------------------------------------------------------
template<typename tTreeNode>
class TreeChildren
{
public:
    typedef tTreeNode TreeNodeType;
    typedef std::list<TreeNodeType*> ListType;
    typedef typename ListType::iterator Iterator;
    typedef typename ListType::const_iterator ConstIterator;
    typedef typename ListType::reverse_iterator ReverseIterator;
    typedef typename ListType::const_reverse_iterator ConstReverseIterator;
    typedef typename ListType::size_type SizeType;

    TreeChildren(TreeNodeType* aOwner)
        : mListImpl()
        , mOwner(aOwner)
    {
    }

    Iterator begin() { return mListImpl.begin(); }
    ConstIterator begin() const { return mListImpl.begin(); }
    Iterator end() { return mListImpl.end(); }
    ConstIterator end() const { return mListImpl.end(); }

    ReverseIterator rbegin() { return mListImpl.rbegin(); }
    ConstReverseIterator rbegin() const { return mListImpl.rbegin(); }
    ReverseIterator rend() { return mListImpl.rend(); }
    ConstReverseIterator rend() const { return mListImpl.rend(); }

    bool empty() const { return mListImpl.empty(); }
    SizeType size() const { return mListImpl.size(); }

    TreeNodeType*& front() { return mListImpl.front(); }
    const TreeNodeType*& front() const { return mListImpl.front(); }
    TreeNodeType*& back() { return mListImpl.back(); }
    const TreeNodeType*& back() const { return mListImpl.back(); }

    Iterator at(int aIndex)
    {
        int index = 0;
        for (Iterator itr = begin(); itr != end(); ++itr, ++index)
            { if (index == aIndex) { return itr; } }
        return end();
    }

    ConstIterator at(int aIndex) const
    {
        int index = 0;
        for (ConstIterator itr = begin(); itr != end(); ++itr, ++index)
            { if (index == aIndex) { return itr; } }
        return end();
    }

    int indexOf(const TreeNodeType* aObj) const
    {
        int index = 0;
        for (ConstIterator itr = begin(); itr != end(); ++itr)
        {
            if (*itr == aObj) return index;
            ++index;
        }
        return -1;
    }

    void pushFront(TreeNodeType* aObj)
    {
        if (!aObj) return;
        aObj->setParent(mOwner);
        aObj->setSibling(nullptr, nullptr);
        if (!mListImpl.empty())
        {
            mListImpl.front()->setPrevSib(aObj);
            aObj->setNextSib(mListImpl.front());
        }
        mListImpl.push_front(aObj);
    }

    TreeNodeType* popFront()
    {
        TreeNodeType* obj = mListImpl.front();
        obj->setParent(nullptr);
        obj->setSibling(nullptr, nullptr);
        mListImpl.pop_front();
        if (!mListImpl.empty())
        {
            mListImpl.front()->setPrevSib(nullptr);
        }
        return obj;
    }

    void pushBack(TreeNodeType* aObj)
    {
        if (!aObj) return;
        aObj->setParent(mOwner);
        aObj->setSibling(nullptr, nullptr);
        if (!mListImpl.empty())
        {
            mListImpl.back()->setNextSib(aObj);
            aObj->setPrevSib(mListImpl.back());
        }
        mListImpl.push_back(aObj);
    }

    TreeNodeType* popBack()
    {
        TreeNodeType* obj = mListImpl.back();
        obj->setParent(nullptr);
        obj->setSibling(nullptr, nullptr);
        mListImpl.pop_back();
        if (!mListImpl.empty())
        {
            mListImpl.back()->setNextSib(nullptr);
        }
        return obj;
    }

    Iterator insert(Iterator aPosition, TreeNodeType* aObj)
    {
        if (!aObj) return end();
        aObj->setParent(mOwner);
        aObj->setSibling(nullptr, nullptr);
        if (!mListImpl.empty())
        {
            if (aPosition != begin())
            {
                Iterator prev = aPosition; --prev;
                aObj->setPrevSib(*prev);
                (*prev)->setNextSib(aObj);
            }
            if (aPosition != end())
            {
                Iterator next = aPosition;
                aObj->setNextSib(*next);
                (*next)->setPrevSib(aObj);
            }
        }
        return mListImpl.insert(aPosition, aObj);
    }

    Iterator erase(Iterator aPosition)
    {
        TreeNodeType* obj = *aPosition;
        obj->setParent(nullptr);
        obj->setSibling(nullptr, nullptr);
        Iterator next = mListImpl.erase(aPosition);
        if (!mListImpl.empty())
        {
            if (next != begin())
            {
                Iterator prev = next; --prev;
                if (next != end())
                {
                    (*prev)->setNextSib(*next);
                    (*next)->setPrevSib(*prev);
                }
                else
                {
                    (*prev)->setNextSib(nullptr);
                }
            }
            else
            {
                (*next)->setPrevSib(nullptr);
            }
        }
        return next;
    }

    void insert(int aIndex, TreeNodeType* aObj)
    {
        insert(at(aIndex), aObj);
    }

    void erase(int aIndex)
    {
        erase(at(aIndex));
    }

    void clear()
    {
        for (Iterator itr = begin(); itr != end(); ++itr)
        {
            TreeNodeType* obj = *itr;
            obj->setParent(nullptr);
            obj->setSibling(nullptr, nullptr);
        }
        mListImpl.clear();
    }

    Iterator find(const TreeNodeType* aObj)
    {
        for (Iterator itr = begin(); itr != end(); ++itr)
        {
            if (*itr == aObj) return itr;
        }
        return end();
    }

    ConstIterator find(const TreeNodeType* aObj) const
    {
        for (ConstIterator itr = begin(); itr != end(); ++itr)
        {
            if (*itr == aObj) return itr;
        }
        return end();
    }

private:
    ListType mListImpl;
    TreeNodeType* mOwner;
};

//-------------------------------------------------------------------------------------------------
template <class tTreeNode>
class TreeNodeBase
{
public:
    typedef tTreeNode TreeNodeType;
    typedef TreeChildren<tTreeNode> Children;


    TreeNodeBase(TreeNodeType* aObj)
        : mParent()
        , mPrevSib()
        , mNextSib()
        , mChildren(aObj)
    {
    }

    virtual ~TreeNodeBase()
    {
    }

    TreeNodeType* parent() { return mParent; }
    const TreeNodeType* parent() const { return mParent; }

    TreeNodeType* prevSib() { return mPrevSib; }
    const TreeNodeType* prevSib() const { return mPrevSib; }

    TreeNodeType* nextSib() { return mNextSib; }
    const TreeNodeType* nextSib() const { return mNextSib; }


    Children& children() { return mChildren; }
    const Children& children() const { return mChildren; }

private:
    friend Children;

    void setParent(TreeNodeType* aParent)
    {
        XC_ASSERT(aParent == nullptr || mParent == nullptr);
        mParent = aParent;
    }

    void setSibling(TreeNodeType* aPrev, TreeNodeType* aNext)
    {
        mPrevSib = aPrev;
        mNextSib = aNext;
    }

    void setPrevSib(TreeNodeType* aSibling)
    {
        mPrevSib = aSibling;
    }

    void setNextSib(TreeNodeType* aSibling)
    {
        mNextSib = aSibling;
    }

    TreeNodeType* mParent;
    TreeNodeType* mPrevSib;
    TreeNodeType* mNextSib;
    Children mChildren;
};

} // namespace util

#endif // UTIL_TREENODEBASE_H
