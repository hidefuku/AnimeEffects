#ifndef UTIL_LIFELINK_H
#define UTIL_LIFELINK_H

#include <list>

namespace util
{

// If you control this class from multiple threads, you need to synchronize it yourself.
class LifeLink
{
public:
    class Node
    {
    public:
        Node()
            : mLink(nullptr)
        {
        }

        Node(const Node& aOther)
            : mLink(nullptr)
        {
            link(aOther.mLink);
        }

        virtual ~Node()
        {
            unlink();
        }

        Node& operator=(LifeLink& aLink)
        {
            link(&aLink);
            return *this;
        }

        Node& operator=(const Node& aOther)
        {
            link(aOther.mLink);
            return *this;
        }

        virtual bool isLinking() const
        {
            return mLink != nullptr;
        }

    private:
        friend class LifeLink;

        void link(LifeLink* aLink)
        {
            unlink();

            if (aLink)
            {
                mLink = aLink;
                mLink->mList.push_back(this);
            }
        }

        void unlink()
        {
            if (mLink)
            {
                mLink->mList.remove(this);
                mLink = nullptr;
            }
        }

        LifeLink* mLink;
    };

    template <typename tObject>
    class Pointee
    {
        typedef void (Pointee::*SafeBoolType)() const;
        void dummyFuncForSafeBoolIdiom() const {}
    public:
        Pointee()
            : lifeLink(nullptr)
            , address(nullptr)
        {
        }

        template<typename tRhs>
        Pointee(const Pointee<tRhs>& aRhs)
            : lifeLink(aRhs.lifeLink)
            , address(aRhs.address)
        {
        }

        template<typename tRhs>
        Pointee& operator=(const Pointee<tRhs>& aRhs)
        {
            lifeLink = aRhs.lifeLink;
            address = aRhs.address;
        }

        Pointee(LifeLink* aLifeLink, tObject* aAddress)
            : lifeLink(aLifeLink)
            , address(aAddress)
        {
        }

        operator SafeBoolType() const
        {
            return lifeLink ? &Pointee::dummyFuncForSafeBoolIdiom : 0;
        }

        LifeLink* lifeLink;
        tObject* address;
    };

    LifeLink()
        : mList()
    {
    }

    LifeLink(const LifeLink& aOther)
        : mList()
    {
        (void)aOther;
    }

    ~LifeLink()
    {
        clear();
    }

    LifeLink& operator=(const LifeLink& aOther)
    {
        (void)aOther;
        clear();
        return *this;
    }

    bool isLinking() const
    {
        return !mList.empty();
    }

    void clear()
    {
        for (NodeList::iterator itr = mList.begin(); itr != mList.end(); ++itr)
        {
            (*itr)->mLink = nullptr;
        }
    }

    template <typename tObject>
    Pointee<tObject> pointee(tObject* aAddress) { return Pointee<tObject>(this, aAddress); }

    template <typename tObject>
    Pointee<const tObject> pointee(const tObject* aAddress) { return Pointee<const tObject>(this, aAddress); }

private:

    typedef std::list<Node*> NodeList;
    NodeList mList;
};

}

#endif // UTIL_LIFELINK_H
